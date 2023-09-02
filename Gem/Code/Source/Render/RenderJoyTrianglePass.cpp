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

#include <Atom/RPI.Reflect/Pass/FullscreenTrianglePassData.h>
#include <Atom/RPI.Reflect/Pass/PassTemplate.h>
#include <Atom/RPI.Reflect/Shader/ShaderAsset.h>

#include <Atom/RHI/Factory.h>
#include <Atom/RHI/FrameScheduler.h>
#include <Atom/RHI/PipelineState.h>

#include <RenderJoy/IRenderJoySrgDataProvider.h>
#include "RenderJoyTrianglePass.h"

namespace RenderJoy
{
    static AZ::RHI::Size GetImageDimensions(AZ::Data::Instance<AZ::RPI::StreamingImage> streamingImage)
    {
        return streamingImage->GetRHIImage()->GetDescriptor().m_size;
    }

    AZ::RPI::Ptr<RenderJoyTrianglePass> RenderJoyTrianglePass::Create(const AZ::RPI::PassDescriptor& descriptor)
    {
        AZ::RPI::Ptr<RenderJoyTrianglePass> pass = aznew RenderJoyTrianglePass(descriptor);
        return pass;
    }

    RenderJoyTrianglePass::RenderJoyTrianglePass(const AZ::RPI::PassDescriptor& descriptor)
        : AZ::RPI::RenderPass(descriptor)
        , m_passDescriptor(descriptor)
    {
        m_entityId = RenderJoyPassRequestBusUtils::GetEntityIdFromPassName(GetName());
        AZ_Assert(m_entityId.IsValid(), "A RenderJoy entity with name %s is supposed to exist", GetName().GetCStr());

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
            else if (RenderJoyPassRequestBusUtils::IsRenderJoyTextureProvider(entityId))
            {
                RenderJoyTextureProviderNotificationBus::MultiHandler::BusConnect(entityId);
            }
            ++channelIndex;
        }

        LoadShader();
        RenderJoyPassNotificationBus::Handler::BusConnect(m_entityId);
    }

    RenderJoyTrianglePass::~RenderJoyTrianglePass()
    {
        RenderJoyTextureProviderNotificationBus::MultiHandler::BusDisconnect();
        RenderJoyPassNotificationBus::Handler::BusDisconnect();
        AZ::RPI::ShaderReloadNotificationBus::Handler::BusDisconnect();
    }

    ///////////////////////////////////////////////////////////////////
    // ShaderReloadNotificationBus overrides...
    void RenderJoyTrianglePass::OnShaderReinitialized(const AZ::RPI::Shader&)
    {
        LoadShader();
    }

    void RenderJoyTrianglePass::OnShaderAssetReinitialized(const AZ::Data::Asset<AZ::RPI::ShaderAsset>&)
    {
        LoadShader();
    }

    void RenderJoyTrianglePass::OnShaderVariantReinitialized(const AZ::RPI::ShaderVariant&)
    {
        LoadShader();
    }
    ///////////////////////////////////////////////////////////////////

    void RenderJoyTrianglePass::LoadShader()
    {
        // Load FullscreenTrianglePassData
        const AZ::RPI::FullscreenTrianglePassData* passData = AZ::RPI::PassUtils::GetPassData<AZ::RPI::FullscreenTrianglePassData>(m_passDescriptor);
        if (passData == nullptr)
        {
            AZ_Error(LogName, false, "[RenderJoyTrianglePass '%s']: Trying to construct without valid FullscreenTrianglePassData!",
                GetPathName().GetCStr());
            return;
        }

        // Load Shader
        AZ::Data::Asset<AZ::RPI::ShaderAsset> shaderAsset;
        if (passData->m_shaderAsset.m_assetId.IsValid())
        {
            shaderAsset = AZ::RPI::FindShaderAsset(passData->m_shaderAsset.m_assetId, passData->m_shaderAsset.m_filePath);
        }

        if (!shaderAsset.GetId().IsValid())
        {
            AZ_Error(LogName, false, "[RenderJoyTrianglePass '%s']: Failed to load shader '%s'!",
                GetPathName().GetCStr(),
                passData->m_shaderAsset.m_filePath.data());
            return;
        }

        m_shader = AZ::RPI::Shader::FindOrCreate(shaderAsset);
        if (m_shader == nullptr)
        {
            AZ_Error(LogName, false, "[RenderJoyTrianglePass '%s']: Failed to load shader '%s'!",
                GetPathName().GetCStr(),
                passData->m_shaderAsset.m_filePath.data());
            return;
        }

        // Load Pass SRG
        const auto& passSrgLayout = m_shader->FindShaderResourceGroupLayout(AZ::Name{ "PassSrg" });
        if (passSrgLayout)
        {
            m_shaderResourceGroup = AZ::RPI::ShaderResourceGroup::Create(shaderAsset, m_shader->GetSupervariantIndex(), passSrgLayout->GetName());

            AZ_Assert(m_shaderResourceGroup, "[RenderJoyTrianglePass '%s']: Failed to create SRG from shader asset '%s'",
                GetPathName().GetCStr(),
                passData->m_shaderAsset.m_filePath.c_str());

            AZ::RPI::PassUtils::BindDataMappingsToSrg(m_passDescriptor, m_shaderResourceGroup.get());
            CacheRenderJoySrgConstantsIndices();
        }

        // Store stencil reference value for the draw call
        m_stencilRef = aznumeric_cast<uint8_t>(passData->m_stencilRef);

        m_initialized = false;

        AZ::RPI::ShaderReloadNotificationBus::Handler::BusDisconnect();
        AZ::RPI::ShaderReloadNotificationBus::Handler::BusConnect(shaderAsset.GetId());
    }

    void RenderJoyTrianglePass::Init()
    {
        // This draw item purposefully does not reference any geometry buffers.
        // Instead it's expected that the extended class uses a vertex shader 
        // that generates a full-screen triangle completely from vertex ids.
        AZ::RHI::DrawLinear draw = AZ::RHI::DrawLinear();
        draw.m_vertexCount = 3;

        AZ::RHI::PipelineStateDescriptorForDraw pipelineStateDescriptor;

        // [GFX TODO][ATOM-872] The pass should be able to drive the shader variant
        // This is a pattern that should be established somewhere.
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

    void RenderJoyTrianglePass::FrameBeginInternal(AZ::RPI::Pass::FramePrepareParams params)
    {
        if (!m_initialized)
        {
            Init();
        }

        const AZ::RPI::PassAttachment* outputAttachment = nullptr;

        if (GetOutputCount() > 0)
        {
            outputAttachment = GetOutputBinding(0).GetAttachment().get();
        }
        else if (GetInputOutputCount() > 0)
        {
            outputAttachment = GetInputOutputBinding(0).GetAttachment().get();
        }

        AZ_Assert(outputAttachment != nullptr, "[RenderJoyTrianglePass %s] has no valid output or input/output attachments.", GetPathName().GetCStr());

        AZ_Assert(outputAttachment->GetAttachmentType() == AZ::RHI::AttachmentType::Image,
            "[RenderJoyTrianglePass %s] output of RenderJoyTrianglePass must be an image", GetPathName().GetCStr());

        AZ::RHI::Size targetImageSize = outputAttachment->m_descriptor.m_image.m_size;

        // Base viewport and scissor off of target attachment
        const float viewWidth = static_cast<float>(targetImageSize.m_width);
        const float viewHeight = static_cast<float>(targetImageSize.m_height);
        m_viewportState = AZ::RHI::Viewport(0, viewWidth, 0, viewHeight);
        m_scissorState = AZ::RHI::Scissor(0, 0, targetImageSize.m_width, targetImageSize.m_height);

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

        RenderPass::FrameBeginInternal(params);
    }

    void RenderJoyTrianglePass::BuildInternal()
    {
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

    void RenderJoyTrianglePass::SetupFrameGraphDependencies(AZ::RHI::FrameGraphInterface frameGraph)
    {
        AZ::RPI::RenderPass::SetupFrameGraphDependencies(frameGraph);
        frameGraph.SetEstimatedItemCount(2);
    }

    void RenderJoyTrianglePass::CompileResources(const AZ::RHI::FrameGraphCompileContext& context)
    {
        if (m_shaderResourceGroup != nullptr)
        {
            BindPassSrg(context, m_shaderResourceGroup);
            m_shaderResourceGroup->Compile();
        }
        if (m_inputChannelIndexForPrevFrameOutputAsInput != InvalidInputChannelIndex)
        {
            SetupCopyImageItem(context);
        }
    }

    // RenderPass functions...
    void RenderJoyTrianglePass::BuildCommandListInternal(const AZ::RHI::FrameGraphExecuteContext& context)
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

    void RenderJoyTrianglePass::Invalidate()
    {
        m_initialized = false;
    }

    void RenderJoyTrianglePass::CacheRenderJoySrgConstantsIndices()
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
                if ((m_entityId == entityId) || !RenderJoyPassRequestBusUtils::IsRenderJoyTextureProvider(entityId))
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
                bool isImmutable = false;
                RenderJoyTextureProviderBus::EventResult(isImmutable, entityId, &RenderJoyTextureProvider::IsImmutable);

                if (isImmutable)
                {
                    AZ::Data::Asset<AZ::RPI::StreamingImageAsset> imageAsset;
                    RenderJoyTextureProviderBus::EventResult(imageAsset, entityId, &RenderJoyTextureProvider::GetStreamingImageAsset);
                    if (imageAsset && imageAsset.IsReady())
                    {
                        const bool success = gotValidTextureProvider = SetImageAssetForChannel(imageIdx, imageAsset);
                        AZ_Error(LogName, success, "Failed to instantiate streaming image for channel %u from asset %s. Will try default image.",
                            imageIdx, imageAsset.GetHint().c_str());
                    }
                }
                else
                {
                    gotValidTextureProvider = SetMutableImageForChannel(entityId, imageIdx);
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

    bool RenderJoyTrianglePass::SetImageAssetForChannel(uint32_t channelIndex, AZ::Data::Asset<AZ::RPI::StreamingImageAsset> imageAsset)
    {
        auto streamingImageInstance = AZ::Data::InstanceDatabase<AZ::RPI::StreamingImage>::Instance().FindOrCreate(imageAsset);
        if (!!streamingImageInstance)
        {
            m_imageChannels[channelIndex] = streamingImageInstance;
            AZ::RHI::Size imageSize = GetImageDimensions(streamingImageInstance);
            m_imageChannelResolutions[channelIndex] = AZ::Vector4(aznumeric_cast<float>(imageSize.m_width), aznumeric_cast<float>(imageSize.m_height), 1.0f, 0.0f);
            return true;
        }
        return false;
    }

    bool RenderJoyTrianglePass::SetMutableImageForChannel(AZ::EntityId entityId, uint32_t channelIndex)
    {
        const void* pixels = nullptr;
        RenderJoyTextureProviderBus::EventResult(pixels, entityId, &RenderJoyTextureProvider::GetPixelBuffer);
        if (!pixels)
        {
            return false;
        }

        AZ::RHI::Format pixelFormat = AZ::RHI::Format::Unknown;
        RenderJoyTextureProviderBus::EventResult(pixelFormat, entityId, &RenderJoyTextureProvider::GetPixelFormat);
        if (pixelFormat == AZ::RHI::Format::Unknown)
        {
            return false;
        }

        AZ::RHI::Size size;
        RenderJoyTextureProviderBus::EventResult(size, entityId, &RenderJoyTextureProvider::GetImageSize);

        AZ::Data::Instance<AZ::RPI::AttachmentImagePool> pool = AZ::RPI::ImageSystemInterface::Get()->GetSystemAttachmentPool();

        auto imageDesc = AZ::RHI::ImageDescriptor::Create2D(AZ::RHI::ImageBindFlags::ShaderRead, size.m_width, size.m_height, pixelFormat);

        // create the image attachment
        auto mutableImageName = AZ::Name(AZStd::string::format("%s_mutable_%u", GetName().GetCStr(), channelIndex));
        AZ::RHI::ClearValue clearValue = AZ::RHI::ClearValue::CreateVector4Float(0, 0, 0, 0);
        AZ::Data::Instance<AZ::RPI::AttachmentImage> attachmentImage = AZ::RPI::AttachmentImage::Create(*pool.get(), imageDesc, mutableImageName, &clearValue, nullptr);
        m_imageChannels[channelIndex] = attachmentImage;
        m_imageChannelResolutions[channelIndex] = AZ::Vector4(aznumeric_cast<float>(size.m_width), aznumeric_cast<float>(size.m_height), 1.0f, 0.0f);

        UpdatePixelDataForChannel(channelIndex, pixels, size, AZ::RHI::GetFormatSize(pixelFormat) * size.m_width);

        return true;
    }

    void RenderJoyTrianglePass::SetDefaultImageForChannel(uint32_t imageChannelIdx)
    {
        AZStd::string imageName = AZStd::string::format("renderjoy_m_channel%u", imageChannelIdx);
        uint32_t color;
        switch (imageChannelIdx)
        {
        case 0: color = 0xFF0000FF; break; //Red
        case 1: color = 0xFF00FF00; break; //Green
        case 2: color = 0xFFFF0000; break; //Blue
        default: color = 0xFF808080; break; //Gray
        }
        CreateImageForChannel(imageChannelIdx, imageName, 256, 256, color);
    }

    void RenderJoyTrianglePass::CreateImageForChannel(uint32_t imageChannelIdx, const AZStd::string& imageName, size_t width, size_t height, uint32_t color)
    {
        auto streamingImageInstance = AZ::Data::InstanceDatabase<AZ::RPI::StreamingImage>::Instance().Find(AZ::Data::InstanceId::CreateName(imageName.c_str()));
        if (!!streamingImageInstance)
        {
            AZ::RHI::Size imageSize = GetImageDimensions(streamingImageInstance);
            m_imageChannels[imageChannelIdx] = streamingImageInstance;
            m_imageChannelResolutions[imageChannelIdx] = AZ::Vector4(aznumeric_cast<float>(imageSize.m_width), aznumeric_cast<float>(imageSize.m_height), 1.0f, 0.0f);
            return;
        }

        AZStd::vector<uint32_t> buffer;
        size_t bufferSize = width * height;
        buffer.resize(bufferSize, color);
        uint8_t* pixels = static_cast<uint8_t*>(static_cast<void*>(&buffer[0]));
        const size_t pixelDataSize = width * height * sizeof(color);

        AZ::RHI::Size imageSize;
        imageSize.m_width = aznumeric_cast<uint32_t>(width);
        imageSize.m_height = aznumeric_cast<uint32_t>(height);

        AZ::Data::Instance<AZ::RPI::StreamingImagePool> streamingImagePool = AZ::RPI::ImageSystemInterface::Get()->GetSystemStreamingPool();

        // CreateFromCpuData will add the image to the instance database.
        m_imageChannels[imageChannelIdx] = AZ::RPI::StreamingImage::CreateFromCpuData(*streamingImagePool, AZ::RHI::ImageDimension::Image2D, imageSize, AZ::RHI::Format::R8G8B8A8_UNORM_SRGB, pixels, pixelDataSize, AZ::Uuid::CreateName(imageName.c_str()));
        m_imageChannelResolutions[imageChannelIdx] = AZ::Vector4(aznumeric_cast<float>(width), aznumeric_cast<float>(height), 1.0f, 0.0f);
    }

    void RenderJoyTrianglePass::RemoveImageForChannel(uint32_t imageChannelIdx)
    {
        m_imageChannels[imageChannelIdx] = nullptr;
        m_imageChannelResolutions[imageChannelIdx] = AZ::Vector4::CreateZero();
    }

    uint32_t RenderJoyTrianglePass::GetInputChannelIndexFromEntityId(const AZ::EntityId& entityId)
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

    void RenderJoyTrianglePass::SetupCopyImageItem(const AZ::RHI::FrameGraphCompileContext& context)
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

    void RenderJoyTrianglePass::UpdatePixelDataForChannel(
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
    void RenderJoyTrianglePass::OnOutputPassChanged([[maybe_unused]] bool isOutputPass)
    {

    }

    void RenderJoyTrianglePass::OnShaderAssetChanged([[maybe_unused]] AZ::Data::Asset<AZ::RPI::ShaderAsset> newShaderAsset)
    {

    }

    void RenderJoyTrianglePass::OnInputChannelEntityChanged(
        [[maybe_unused]] uint32_t inputChannelIndex, [[maybe_unused]] AZ::EntityId newEntityId)
    {

    }
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // RenderJoyTextureProviderNotificationBus overrides...
    void RenderJoyTrianglePass::OnStreamingImageAssetChanged(AZ::Data::Asset<AZ::RPI::StreamingImageAsset> imageAsset)
    {
        const AZ::EntityId* entityId = RenderJoyTextureProviderNotificationBus::GetCurrentBusId();
        auto channelIndex = GetInputChannelIndexFromEntityId(*entityId);

        if (!imageAsset || !imageAsset.IsReady())
        {
            //Set the default image
            SetDefaultImageForChannel(channelIndex);
        }
        else
        {
            SetImageAssetForChannel(channelIndex, imageAsset);
        }
        m_shaderResourceGroup->SetImage(m_imageChannelsIndex, m_imageChannels[channelIndex], channelIndex);
        m_shaderResourceGroup->SetConstant<AZ::Vector4>(
            m_imageChannelResolutionsIndex, m_imageChannelResolutions[channelIndex], channelIndex);
    }

    void RenderJoyTrianglePass::OnPixelBufferChanged(const void* pixels, AZ::RHI::Size imageSize, uint32_t bytesPerRow)
    {
        const AZ::EntityId* entityId = RenderJoyTextureProviderNotificationBus::GetCurrentBusId();
        auto channelIndex = GetInputChannelIndexFromEntityId(*entityId);

        if (!pixels)
        {
            // Set the default image
            SetDefaultImageForChannel(channelIndex);
        }
        else
        {
            UpdatePixelDataForChannel(channelIndex, pixels, imageSize, bytesPerRow);
        }
        m_shaderResourceGroup->SetImage(m_imageChannelsIndex, m_imageChannels[channelIndex], channelIndex);
        m_shaderResourceGroup->SetConstant<AZ::Vector4>(
            m_imageChannelResolutionsIndex, m_imageChannelResolutions[channelIndex], channelIndex);
    }
    ///////////////////////////////////////////////////////////////////

}   // namespace RenderJoy
