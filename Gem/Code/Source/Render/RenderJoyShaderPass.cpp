/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Asset/AssetManagerBus.h>

#include <AtomCore/Instance/InstanceDatabase.h>

#include <Atom/RPI.Public/Pass/PassUtils.h>
#include <Atom/RPI.Public/RPIUtils.h>
#include <Atom/RPI.Public/Image/AttachmentImagePool.h>
#include <Atom/RPI.Public/Image/StreamingImagePool.h>
#include <Atom/RPI.Public/Image/ImageSystemInterface.h>

#include <Atom/RPI.Reflect/Pass/PassTemplate.h>
#include <Atom/RPI.Reflect/Shader/ShaderAsset.h>

#include <Atom/RHI/Factory.h>
#include <Atom/RHI/FrameScheduler.h>
#include <Atom/RHI/PipelineState.h>

#include <RenderJoy/RenderJoyBus.h>
#include <RenderJoy/IRenderJoySrgDataProvider.h>
#include <Render/RenderJoyShaderPassData.h>
#include "RenderJoyShaderPass.h"

namespace RenderJoy
{
    static AZ::RHI::Size GetImageDimensions(AZ::Data::Instance<AZ::RPI::Image> image)
    {
        return image->GetRHIImage()->GetDescriptor().m_size;
    }

    AZ::RPI::Ptr<RenderJoyShaderPass> RenderJoyShaderPass::Create(const AZ::RPI::PassDescriptor& descriptor)
    {
        AZ::RPI::Ptr<RenderJoyShaderPass> pass = aznew RenderJoyShaderPass(descriptor);
        return pass;
    }

    RenderJoyShaderPass::RenderJoyShaderPass(const AZ::RPI::PassDescriptor& descriptor)
        : AZ::RPI::RenderPass(descriptor)
        , m_passDescriptor(descriptor)
    {
        auto renderJoySystem = RenderJoyInterface::Get();
        m_entityId = renderJoySystem->GetEntityIdFromPassName(GetName());
        AZ_Assert(m_entityId.IsValid(), "A RenderJoy entity from pass name %s is supposed to exist", GetName().GetCStr());

        m_renderJoySrgDataProvider = AZ::Interface<IRenderJoySrgDataProvider>::Get();
        AZ_Assert(!!m_renderJoySrgDataProvider, "What happened with IRenderJoySrgDataProvider?");

        AZStd::vector<AZ::EntityId> entitiesOnInputChannels;
        RenderJoyPassRequestBus::EventResult(entitiesOnInputChannels, m_entityId, &RenderJoyPassRequests::GetEntitiesOnInputChannels);
        uint32_t channelIndex = 0;
        for (const auto& entityId : entitiesOnInputChannels)
        {
            m_entitiesOnInputChannels[channelIndex] = entityId;
            if (m_entityId == entityId)
            {
                m_inputChannelIndexForPrevFrameOutputAsInput = channelIndex;
            }
            else if (Utils::IsRenderJoyTextureProvider(entityId))
            {
                RenderJoyTextureProviderNotificationBus::MultiHandler::BusConnect(entityId);
            }
            ++channelIndex;
        }

        const RenderJoyShaderPassData* passData = AZ::RPI::PassUtils::GetPassData<RenderJoyShaderPassData>(m_passDescriptor);
        m_renderTargetWidth = passData->m_renderTargetWidth;
        m_renderTargetHeight = passData->m_renderTargetHeight;
        AZ_Assert(m_renderTargetWidth && m_renderTargetHeight, "Invalid render target width or height from RenderJoyShaderPassData");

        LoadShader();
        RenderJoyPassNotificationBus::Handler::BusConnect(m_entityId);
    }

    RenderJoyShaderPass::~RenderJoyShaderPass()
    {
        RenderJoyTextureProviderNotificationBus::MultiHandler::BusDisconnect();
        RenderJoyPassNotificationBus::Handler::BusDisconnect();
        AZ::RPI::ShaderReloadNotificationBus::Handler::BusDisconnect();
    }

    ///////////////////////////////////////////////////////////////////
    // ShaderReloadNotificationBus overrides...
    void RenderJoyShaderPass::OnShaderReinitialized(const AZ::RPI::Shader&)
    {
        LoadShader();
    }

    void RenderJoyShaderPass::OnShaderAssetReinitialized(const AZ::Data::Asset<AZ::RPI::ShaderAsset>&)
    {
        LoadShader();
    }

    void RenderJoyShaderPass::OnShaderVariantReinitialized(const AZ::RPI::ShaderVariant&)
    {
        LoadShader();
    }
    ///////////////////////////////////////////////////////////////////

    void RenderJoyShaderPass::LoadShader()
    {
        // Load FullscreenTrianglePassData
        const RenderJoyShaderPassData* passData = AZ::RPI::PassUtils::GetPassData<RenderJoyShaderPassData>(m_passDescriptor);
        if (passData == nullptr)
        {
            AZ_Error(ClassNameStr, false, "['%s']: Trying to construct without valid RenderJoyShaderPassData!",
                GetPathName().GetCStr());
            return;
        }

        // Load Shader
        if (!m_shaderAsset.IsReady())
        {
            AZ::Data::Asset<AZ::RPI::ShaderAsset> shaderAsset;
            if (passData->m_shaderAsset.m_assetId.IsValid())
            {
                shaderAsset = AZ::RPI::FindShaderAsset(passData->m_shaderAsset.m_assetId, passData->m_shaderAsset.m_filePath);
            }

            if (!shaderAsset.GetId().IsValid())
            {
                AZ_Error(ClassNameStr, false, "['%s']: Failed to load shader '%s'!",
                    GetPathName().GetCStr(),
                    passData->m_shaderAsset.m_filePath.data());
                return;
            }
            m_shaderAsset = shaderAsset;
        }

        m_shader = AZ::RPI::Shader::FindOrCreate(m_shaderAsset);
        if (m_shader == nullptr)
        {
            AZ_Error(ClassNameStr, false, "['%s']: Failed to load shader '%s'!",
                GetPathName().GetCStr(),
                passData->m_shaderAsset.m_filePath.data());
            return;
        }

        // Load Pass SRG
        const auto& passSrgLayout = m_shader->FindShaderResourceGroupLayout(AZ::Name{ "PassSrg" });
        if (passSrgLayout)
        {
            m_shaderResourceGroup = AZ::RPI::ShaderResourceGroup::Create(m_shaderAsset, m_shader->GetSupervariantIndex(), passSrgLayout->GetName());

            AZ_Assert(m_shaderResourceGroup, "['%s']: Failed to create SRG from shader asset '%s'",
                GetPathName().GetCStr(),
                passData->m_shaderAsset.m_filePath.c_str());

            AZ::RPI::PassUtils::BindDataMappingsToSrg(m_passDescriptor, m_shaderResourceGroup.get());
            CacheRenderJoySrgConstantsIndices();
        }

        // Store stencil reference value for the draw call
        m_stencilRef = aznumeric_cast<uint8_t>(passData->m_stencilRef);

        m_initialized = false;

        AZ::RPI::ShaderReloadNotificationBus::Handler::BusDisconnect();
        AZ::RPI::ShaderReloadNotificationBus::Handler::BusConnect(m_shaderAsset.GetId());
    }

    void RenderJoyShaderPass::Init()
    {
        // This draw item purposefully does not reference any geometry buffers.
        // Instead it's expected that the extended class uses a vertex shader 
        // that generates a full-screen triangle completely from vertex ids.
        AZ::RHI::DrawLinear draw = AZ::RHI::DrawLinear();
        draw.m_vertexCount = 3;

        AZ::RHI::PipelineStateDescriptorForDraw pipelineStateDescriptor;

        auto shaderVariant = m_shader->GetVariant(AZ::RPI::RootShaderVariantStableId);
        shaderVariant.ConfigurePipelineState(pipelineStateDescriptor);

        pipelineStateDescriptor.m_renderAttachmentConfiguration = GetRenderAttachmentConfiguration();
        pipelineStateDescriptor.m_renderStates.m_multisampleState = GetMultisampleState();

        // No streams required
        AZ::RHI::InputStreamLayout inputStreamLayout;
        inputStreamLayout.SetTopology(AZ::RHI::PrimitiveTopology::TriangleList);
        inputStreamLayout.Finalize();

        pipelineStateDescriptor.m_inputStreamLayout = inputStreamLayout;

        m_item.m_arguments = AZ::RHI::DrawArguments(draw);
        m_item.m_pipelineState = m_shader->AcquirePipelineState(pipelineStateDescriptor);
        m_item.m_stencilRef = m_stencilRef;

        m_initialized = true;
    }

    void RenderJoyShaderPass::FrameBeginInternal(AZ::RPI::Pass::FramePrepareParams params)
    {
        if (!m_initialized)
        {
            Init();
        }

        RenderPass::FrameBeginInternal(params);
    }

    void RenderJoyShaderPass::BuildInternal()
    {
        // Base viewport and scissor off of target attachment
        const float viewWidth = static_cast<float>(m_renderTargetWidth);
        const float viewHeight = static_cast<float>(m_renderTargetHeight);
        m_viewportState = AZ::RHI::Viewport(0, viewWidth, 0, viewHeight);
        m_scissorState = AZ::RHI::Scissor(0, 0, m_renderTargetWidth, m_renderTargetHeight);

        if (m_inputChannelIndexForPrevFrameOutputAsInput == InvalidInputChannelIndex)
        {
            return;
        }

        AZ::Data::Instance<AZ::RPI::AttachmentImagePool> pool = AZ::RPI::ImageSystemInterface::Get()->GetSystemAttachmentPool();

        AZ::RPI::Ptr<AZ::RPI::PassAttachment> prevFrameImageAttachment = FindOwnedAttachment(AZ::Name("PreviousFrameImage"));
        AZ_Assert(prevFrameImageAttachment.get(), "Failed to get PreviousFrameImage attachment");

        // update the image attachment descriptor to sync up size and format
        prevFrameImageAttachment->Update();

        AZ::RHI::ImageDescriptor& imageDesc = prevFrameImageAttachment->m_descriptor.m_image;

        // change the lifetime since we want it to live between frames
        prevFrameImageAttachment->m_lifetime = AZ::RHI::AttachmentLifetimeType::Imported;

        // set the bind flags
        imageDesc.m_bindFlags |= AZ::RHI::ImageBindFlags::ShaderRead | AZ::RHI::ImageBindFlags::CopyWrite;

        // create the image attachment
        AZ::RHI::ClearValue clearValue = AZ::RHI::ClearValue::CreateVector4Float(0, 1, 0, 0);
        m_prevFrameOutputAsInput = AZ::RPI::AttachmentImage::Create(
            *pool.get(), imageDesc, AZ::Name(prevFrameImageAttachment->m_path.GetCStr()), &clearValue, nullptr);

        prevFrameImageAttachment->m_path = m_prevFrameOutputAsInput->GetAttachmentId();
        prevFrameImageAttachment->m_importedResource = m_prevFrameOutputAsInput;

    }

    // Scope producer functions

    void RenderJoyShaderPass::SetupFrameGraphDependencies(AZ::RHI::FrameGraphInterface frameGraph)
    {
        AZ::RPI::RenderPass::SetupFrameGraphDependencies(frameGraph);
        frameGraph.SetEstimatedItemCount(2);
    }

    void RenderJoyShaderPass::CompileResources(const AZ::RHI::FrameGraphCompileContext& context)
    {
        // Base viewport and scissor off of target attachment
        const float viewWidth = static_cast<float>(m_renderTargetWidth);
        const float viewHeight = static_cast<float>(m_renderTargetHeight);

        if (m_resolutionIndex.IsValid())
        {
            m_shaderResourceGroup->SetConstant<AZ::Vector3>(m_resolutionIndex, AZ::Vector3(viewWidth, viewHeight, 1.0f));
        }
        if (m_renderJoySrgDataProvider)
        {
            if (m_timeIndex.IsValid())
            {
                m_shaderResourceGroup->SetConstant<float>(m_timeIndex, m_renderJoySrgDataProvider->GetTime());
            }
            if (m_timeDeltaIndex.IsValid())
            {
                m_shaderResourceGroup->SetConstant<float>(m_timeDeltaIndex, m_renderJoySrgDataProvider->GetTimeDelta());
            }
            if (m_frameIndex.IsValid())
            {
                m_shaderResourceGroup->SetConstant<int>(m_frameIndex, m_renderJoySrgDataProvider->GetFramesCount());
            }
            if (m_frameRateIndex.IsValid())
            {
                m_shaderResourceGroup->SetConstant<float>(m_frameRateIndex, m_renderJoySrgDataProvider->GetFramesPerSecond());
            }
            if (m_mouseIndex.IsValid())
            {
                AZ::Vector2 currentPos;
                AZ::Vector2 clickPos;
                bool isLeftButtonDown;
                bool isLeftButtonClick;
                m_renderJoySrgDataProvider->GetMouseData(currentPos, clickPos, isLeftButtonDown, isLeftButtonClick);
                AZ::Vector4 encodedMouseData(currentPos.GetX(), currentPos.GetY(),
                    isLeftButtonDown ? clickPos.GetX() : -clickPos.GetX(),
                    isLeftButtonClick ? clickPos.GetY() : -clickPos.GetY());
                m_shaderResourceGroup->SetConstant<AZ::Vector4>(m_mouseIndex, encodedMouseData);
            }
        }

        BindPassSrg(context, m_shaderResourceGroup);
        m_shaderResourceGroup->Compile();

        if (m_inputChannelIndexForPrevFrameOutputAsInput != InvalidInputChannelIndex)
        {
            SetupCopyImageItem(context);
        }
    }

    // RenderPass functions...
    void RenderJoyShaderPass::BuildCommandListInternal(const AZ::RHI::FrameGraphExecuteContext& context)
    {
        AZ::RHI::CommandList* commandList = context.GetCommandList();

        SetSrgsForDraw(commandList);

        commandList->SetViewport(m_viewportState);
        commandList->SetScissor(m_scissorState);

        commandList->Submit(m_item);

        if (m_inputChannelIndexForPrevFrameOutputAsInput != InvalidInputChannelIndex)
        {
            commandList->Submit(m_currentOutputCopyItem);
        }
    }

    void RenderJoyShaderPass::Invalidate()
    {
        m_initialized = false;
    }

    void RenderJoyShaderPass::CacheRenderJoySrgConstantsIndices()
    {
        AZ_Assert(!!m_shaderResourceGroup, "Don't call this if there's no PassSrg");
        const auto srgLayout = m_shaderResourceGroup->GetLayout();
        m_resolutionIndex = srgLayout->FindShaderInputConstantIndex(AZ::Name{ "m_resolution" });
        m_timeIndex = srgLayout->FindShaderInputConstantIndex(AZ::Name{ "m_time" });
        m_timeDeltaIndex = srgLayout->FindShaderInputConstantIndex(AZ::Name{ "m_timeDelta" });

        m_frameIndex = srgLayout->FindShaderInputConstantIndex(AZ::Name{"m_frame"});
        m_frameRateIndex = srgLayout->FindShaderInputConstantIndex(AZ::Name{"m_frameRate"});

        m_mouseIndex = srgLayout->FindShaderInputConstantIndex(AZ::Name{ "m_mouse" });

        m_imageChannelResolutionsIndex = srgLayout->FindShaderInputConstantIndex(AZ::Name("m_channelResolution"));
        m_imageChannelsIndex = srgLayout->FindShaderInputImageIndex(AZ::Name("m_channel"));
        if (m_imageChannelsIndex.IsValid())
        {
            for (uint32_t imageIdx = 0; imageIdx < ImageChannelsCount; ++imageIdx)
            {
                auto entityId = m_entitiesOnInputChannels[imageIdx];
                if ((m_entityId == entityId) || !Utils::IsRenderJoyTextureProvider(entityId))
                {
                    continue;
                }

                if (!srgLayout->ValidateAccess(m_imageChannelsIndex, imageIdx))
                {
                    RemoveImageForChannel(imageIdx);
                    continue;
                }

                //Get the image first from the data provider (if available)
                bool gotValidTextureProvider = false;
                AZ::Data::Instance<AZ::RPI::Image> image;
                RenderJoyTextureProviderBus::EventResult(image, entityId, &RenderJoyTextureProvider::GetImage);
                if (image)
                {
                    const bool success = gotValidTextureProvider = SetImageForChannel(imageIdx, image);
                    AZ_Error(ClassNameStr, success, "Failed to get image for channel %u from provider %s. Will try default image.",
                        imageIdx, entityId.ToString().c_str());
                }
                if (!gotValidTextureProvider)
                {
                    SetDefaultImageForChannel(imageIdx);
                }

                m_shaderResourceGroup->SetImage(m_imageChannelsIndex, m_imageChannels[imageIdx], imageIdx);
                m_shaderResourceGroup->SetConstant<AZ::Vector4>(
                    m_imageChannelResolutionsIndex, m_imageChannelResolutions[imageIdx], imageIdx);
            }
        }
    }

    bool RenderJoyShaderPass::SetImageForChannel(uint32_t channelIndex, AZ::Data::Instance<AZ::RPI::Image> image)
    {
        m_imageChannels[channelIndex] = image;
        AZ::RHI::Size imageSize = GetImageDimensions(image);
        m_imageChannelResolutions[channelIndex] = AZ::Vector4(aznumeric_cast<float>(imageSize.m_width), aznumeric_cast<float>(imageSize.m_height), 1.0f, 0.0f);
        return true;
    }

    void RenderJoyShaderPass::SetDefaultImageForChannel(uint32_t imageChannelIdx)
    {
        auto renderJoySystem = RenderJoyInterface::Get();
        auto image = renderJoySystem->GetDefaultInputTexture(imageChannelIdx);
        AZ::RHI::Size imageSize = GetImageDimensions(image);
        m_imageChannels[imageChannelIdx] = image;
        m_imageChannelResolutions[imageChannelIdx] = AZ::Vector4(aznumeric_cast<float>(imageSize.m_width), aznumeric_cast<float>(imageSize.m_height), 1.0f, 0.0f);
    }

    void RenderJoyShaderPass::RemoveImageForChannel(uint32_t imageChannelIdx)
    {
        m_imageChannels[imageChannelIdx] = nullptr;
        m_imageChannelResolutions[imageChannelIdx] = AZ::Vector4::CreateZero();
    }

    uint32_t RenderJoyShaderPass::GetInputChannelIndexFromEntityId(const AZ::EntityId& entityId)
    {
        for (uint32_t channelIndex = 0; channelIndex < ImageChannelsCount; ++channelIndex)
        {
            if (entityId == m_entitiesOnInputChannels[channelIndex])
            {
                return channelIndex;
            }
        }
        AZ_Assert(false, "Never expected to be here for entityId = %s", entityId.ToString().c_str());
        return ImageChannelsCount;
    }

    void RenderJoyShaderPass::SetupCopyImageItem(const AZ::RHI::FrameGraphCompileContext& context)
    {
        AZ::RHI::CopyImageDescriptor copyDesc;

        // Source Image (The render target)
        AZ::RPI::PassAttachmentBinding& copySource = GetOutputBinding(0);
        const AZ::RHI::Image* sourceImage = context.GetImage(copySource.GetAttachment()->GetAttachmentId());
        copyDesc.m_sourceImage = sourceImage;
        copyDesc.m_sourceSize = sourceImage->GetDescriptor().m_size;

        // Destination Image (m_prevFrameOutputAsInput)
        copyDesc.m_destinationImage = context.GetImage(m_prevFrameOutputAsInput->GetAttachmentId());

        m_currentOutputCopyItem = copyDesc;
    }

    void RenderJoyShaderPass::UpdatePixelDataForChannel(
        uint32_t channelIndex, const void* pixels, const AZ::RHI::Size& imageSize, uint32_t bytesPerRow)
    {
        AZ::RHI::ImageUpdateRequest updateRequest;
        auto& image = m_imageChannels[channelIndex];

        updateRequest.m_image = image->GetRHIImage();
        updateRequest.m_sourceData = pixels;

        updateRequest.m_sourceSubresourceLayout.m_size = imageSize;
        updateRequest.m_sourceSubresourceLayout.m_rowCount = imageSize.m_height;
        updateRequest.m_sourceSubresourceLayout.m_bytesPerRow = bytesPerRow;
        updateRequest.m_sourceSubresourceLayout.m_bytesPerImage = bytesPerRow * imageSize.m_height;

        image->UpdateImageContents(updateRequest);
    }

    ///////////////////////////////////////////////////////////////////
    // RenderJoyPassNotificationBus overrides...
    void RenderJoyShaderPass::OnShaderAssetChanged([[maybe_unused]] AZ::Data::Asset<AZ::RPI::ShaderAsset> newShaderAsset)
    {
        AZ::RPI::ShaderReloadNotificationBus::Handler::BusDisconnect();
        m_shaderAsset = newShaderAsset;
        LoadShader();
    }

    void RenderJoyShaderPass::OnInputChannelEntityChanged(
        [[maybe_unused]] uint32_t inputChannelIndex, [[maybe_unused]] AZ::EntityId newEntityId)
    {

    }
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // RenderJoyTextureProviderNotificationBus overrides...
    void RenderJoyShaderPass::OnImageChanged(AZ::Data::Instance<AZ::RPI::Image> image)
    {
        const AZ::EntityId* entityId = RenderJoyTextureProviderNotificationBus::GetCurrentBusId();
        auto channelIndex = GetInputChannelIndexFromEntityId(*entityId);

        if (!image)
        {
            //Set the default image
            SetDefaultImageForChannel(channelIndex);
        }
        else
        {
            SetImageForChannel(channelIndex, image);
        }
        m_shaderResourceGroup->SetImage(m_imageChannelsIndex, m_imageChannels[channelIndex], channelIndex);
        m_shaderResourceGroup->SetConstant<AZ::Vector4>(
            m_imageChannelResolutionsIndex, m_imageChannelResolutions[channelIndex], channelIndex);
    }
    ///////////////////////////////////////////////////////////////////

}   // namespace RenderJoy
