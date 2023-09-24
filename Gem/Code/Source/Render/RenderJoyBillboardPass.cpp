/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <Atom/RPI.Public/Pass/PassUtils.h>
#include <Atom/RPI.Public/RPIUtils.h>
#include <Atom/RPI.Public/Shader/ShaderReloadDebugTracker.h>
#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RPI.Public/Scene.h>

#include <Atom/RPI.Reflect/Pass/PassTemplate.h>
#include <Atom/RPI.Reflect/Shader/ShaderAsset.h>
#include <Atom/RPI.Reflect/Asset/AssetUtils.h>

#include <Atom/RHI/Factory.h>
#include <Atom/RHI/FrameScheduler.h>
#include <Atom/RHI/PipelineState.h>

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Asset/AssetManagerBus.h>
#include <AzCore/std/algorithm.h>

#include <RenderJoy/RenderJoyFeatureProcessorInterface.h>
#include <RenderJoy/RenderJoyBus.h>
#include "RenderJoyBillboardPassData.h"
#include "RenderJoyBillboardPass.h"

namespace RenderJoy
{
    AZ::RPI::Ptr<RenderJoyBillboardPass> RenderJoyBillboardPass::Create(const AZ::RPI::PassDescriptor& descriptor)
    {
        AZ_Printf(PassNameStr, "%s\n", __FUNCTION__);
        AZ::RPI::Ptr<RenderJoyBillboardPass> pass = aznew RenderJoyBillboardPass(descriptor);
        return pass;
    }
    
    RenderJoyBillboardPass::RenderJoyBillboardPass(const AZ::RPI::PassDescriptor& descriptor)
        : RenderPass(descriptor)
        , m_passDescriptor(descriptor)
    {
        m_flatscreenLayout = AZ::Vector4(0.0f, 0.0f, 1.0f, 1.0f);
        LoadShader();
    }
    
    RenderJoyBillboardPass::~RenderJoyBillboardPass()
    {
        AZ_Printf(PassNameStr, "%s\n", __FUNCTION__);
        AZ::RPI::ShaderReloadNotificationBus::Handler::BusDisconnect();
    }
    
    AZ::Data::Instance<AZ::RPI::Shader> RenderJoyBillboardPass::GetShader() const
    {
        return m_shader;
    }
    
    void RenderJoyBillboardPass::OnShaderReinitialized(const AZ::RPI::Shader&)
    {
        LoadShader();
    }
    
    void RenderJoyBillboardPass::OnShaderAssetReinitialized(const AZ::Data::Asset<AZ::RPI::ShaderAsset>&)
    {
        LoadShader();
    }
    
    void RenderJoyBillboardPass::OnShaderVariantReinitialized(const AZ::RPI::ShaderVariant&)
    {
        LoadShader();
    }
    
    void RenderJoyBillboardPass::LoadShader()
    {
        AZ_Assert(GetPassState() != AZ::RPI::PassState::Rendering, "RenderJoyBillboardPass - Reloading shader during Rendering phase!");
    
        // Load RenderJoyBillboardPassData
        const RenderJoyBillboardPassData* passData = AZ::RPI::PassUtils::GetPassData<RenderJoyBillboardPassData>(m_passDescriptor);
        if (passData == nullptr)
        {
            AZ_Error(PassNameStr, false, "[RenderJoyBillboardPass '%s']: Trying to construct without valid RenderJoyBillboardPassData!",
                GetPathName().GetCStr());
            return;
        }
    
        // Load Shader
        AZ::Data::Asset<AZ::RPI::ShaderAsset> shaderAsset;
        AZ::Data::AssetId shaderAssetId = AZ::RPI::AssetUtils::GetAssetIdForProductPath(BillboardShaderProductPath);
        if (shaderAssetId.IsValid())
        {
            shaderAsset = AZ::RPI::FindShaderAsset(shaderAssetId, AZStd::string(BillboardShaderProductPath));
        }
    
        if (!shaderAsset.GetId().IsValid())
        {
            AZ_Error(PassNameStr, false, "[RenderJoyBillboardPass '%s']: Failed to load shader asset '%s'!",
                GetPathName().GetCStr(),
                BillboardShaderProductPath);
            return;
        }
    
        m_shader = AZ::RPI::Shader::FindOrCreate(shaderAsset);
        if (m_shader == nullptr)
        {
            AZ_Error(PassNameStr, false, "[RenderJoyBillboardPass '%s']: Failed to load shader '%s'!",
                GetPathName().GetCStr(),
                BillboardShaderProductPath);
            return;
        }
    
        // Store stencil reference value for the draw call
        m_stencilRef = passData->m_stencilRef;

        m_inputTextureIsAttachment = passData->m_inputTextureIsAttachment;
        if (!m_inputTextureIsAttachment)
        {
            auto renderJoySystem = RenderJoyInterface::Get();
            m_inputTextureImage = renderJoySystem->GetInvalidParentPassTexture();
        }
    
        m_pipelineStateForDraw.Init(m_shader);
    
        CreateSrgs();
    
        QueueForInitialization();

        m_shaderConstantsNeedUpdate = true;
    
        AZ::RPI::ShaderReloadNotificationBus::Handler::BusDisconnect();
        AZ::RPI::ShaderReloadNotificationBus::Handler::BusConnect(shaderAsset.GetId());
    }
    
    void RenderJoyBillboardPass::CreateSrgs()
    {
        if (!m_shader)
        {
            return;
        }
    
        // Load Pass SRG
        const auto passSrgLayout = m_shader->FindShaderResourceGroupLayout(AZ::RPI::SrgBindingSlot::Pass);
        if (passSrgLayout)
        {
            m_shaderResourceGroup = AZ::RPI::ShaderResourceGroup::Create(m_shader->GetAsset(), m_shader->GetSupervariantIndex(), passSrgLayout->GetName());
    
            //[[maybe_unused]] const RenderJoyBillboardPassData* passData = AZ::RPI::PassUtils::GetPassData<RenderJoyBillboardPassData>(m_passDescriptor);
    
            AZ_Assert(m_shaderResourceGroup, "[RenderJoyBillboardPass '%s']: Failed to create SRG from shader asset '%s'",
                GetPathName().GetCStr(),
                BillboardShaderProductPath);
    
            AZ::RPI::PassUtils::BindDataMappingsToSrg(m_passDescriptor, m_shaderResourceGroup.get());
        }
    
        // Load Draw SRG
        // this is necessary since the shader may have options, which require a default draw SRG
        const bool compileDrawSrg = false; // The SRG will be compiled in CompileResources()
        m_drawShaderResourceGroup = m_shader->CreateDefaultDrawSrg(compileDrawSrg);
    
        // It is valid for there to be no draw srg if there are no shader options, so check to see if it is null.
        if (m_drawShaderResourceGroup)
        {
            m_pipelineStateForDraw.UpdateSrgVariantFallback(m_shaderResourceGroup);
        }

        m_shaderConstantsNeedUpdate = true;
    }
    
    void RenderJoyBillboardPass::BuildDrawItem()
    {
        m_pipelineStateForDraw.SetOutputFromPass(this);
    
        // No streams required, the vertex shader automatically
        // defines the billboard coordinates.
        AZ::RHI::InputStreamLayout inputStreamLayout;
        inputStreamLayout.SetTopology(AZ::RHI::PrimitiveTopology::TriangleList);
        inputStreamLayout.Finalize();
    
        m_pipelineStateForDraw.SetInputStreamLayout(inputStreamLayout);
    
        // This draw item purposefully does not reference any geometry buffers.
        // Instead it's expected that the extended class uses a vertex shader 
        // that generates a full-screen triangle completely from vertex ids.
        AZ::RHI::DrawLinear draw = AZ::RHI::DrawLinear();
        draw.m_vertexCount = 6; //6-vertices, as this is a billboard that can be rendered anywhere in the world.
    
        m_item.m_arguments = AZ::RHI::DrawArguments(draw);
        m_item.m_pipelineState = m_pipelineStateForDraw.Finalize();
        m_item.m_stencilRef = static_cast<uint8_t>(m_stencilRef);
    }
    
    void RenderJoyBillboardPass::InitializeInternal()
    {
        AZ::RPI::RenderPass::InitializeInternal();
        
        AZ::RPI::ShaderReloadDebugTracker::ScopedSection reloadSection("{%p}->RenderJoyBillboardPass::InitializeInternal", this);
    
        if (m_shader == nullptr)
        {
            AZ_Error(PassNameStr, false, "[RenderJoyBillboardPass]: Shader not loaded!");
            return;
        }
    
        BuildDrawItem();

        // Notify the feature processor
        AZ::RPI::Scene* scene = m_pipeline->GetScene();
        auto* fp = scene->GetFeatureProcessor<RenderJoyFeatureProcessorInterface>();
        if (fp) // can be null when switching levels.
        {
            fp->OnBillboardPassReady(GetName());
        }

    }
    
    void RenderJoyBillboardPass::FrameBeginInternal(FramePrepareParams params)
    {
        const AZ::RPI::PassAttachment* outputAttachment = nullptr;
        
        if (GetOutputCount() > 0)
        {
            outputAttachment = GetOutputBinding(0).GetAttachment().get();
        }
        else if(GetInputOutputCount() > 0)
        {
            outputAttachment = GetInputOutputBinding(0).GetAttachment().get();
        }
    
        AZ_Assert(outputAttachment != nullptr, "[RenderJoyBillboardPass %s] has no valid output or input/output attachments.", GetPathName().GetCStr());
    
        AZ_Assert(outputAttachment->GetAttachmentType() == AZ::RHI::AttachmentType::Image,
            "[RenderJoyBillboardPass %s] output of RenderJoyBillboardPass must be an image", GetPathName().GetCStr());
    
        AZ::RHI::Size targetImageSize = outputAttachment->m_descriptor.m_image.m_size;
    
        m_viewportState.m_maxX = static_cast<float>(targetImageSize.m_width);
        m_viewportState.m_maxY = static_cast<float>(targetImageSize.m_height);
    
        m_scissorState.m_maxX = static_cast<int32_t>(targetImageSize.m_width);
        m_scissorState.m_maxY = static_cast<int32_t>(targetImageSize.m_height);
    
        AZ::RPI::RenderPass::FrameBeginInternal(params);
    }
    
    // Scope producer functions
    
    void RenderJoyBillboardPass::SetupFrameGraphDependencies(AZ::RHI::FrameGraphInterface frameGraph)
    {
        RenderPass::SetupFrameGraphDependencies(frameGraph);
    
        // Update scissor/viewport regions based on the mip level of the render target that is being written into
        uint16_t viewMinMip = AZ::RHI::ImageSubresourceRange::HighestSliceIndex;
        for (const AZ::RPI::PassAttachmentBinding& attachmentBinding : m_attachmentBindings)
        {
            if (attachmentBinding.GetAttachment() != nullptr &&
                frameGraph.GetAttachmentDatabase().IsAttachmentValid(attachmentBinding.GetAttachment()->GetAttachmentId()) &&
                attachmentBinding.m_unifiedScopeDesc.GetType() == AZ::RHI::AttachmentType::Image &&
                AZ::RHI::CheckBitsAny(attachmentBinding.GetAttachmentAccess(), AZ::RHI::ScopeAttachmentAccess::Write) &&
                attachmentBinding.m_scopeAttachmentUsage == AZ::RHI::ScopeAttachmentUsage::RenderTarget)
            {
                AZ::RHI::ImageViewDescriptor viewDesc = attachmentBinding.m_unifiedScopeDesc.GetAsImage().m_imageViewDescriptor;
                viewMinMip = AZStd::min(viewMinMip, viewDesc.m_mipSliceMin);
            }
        }
        
        if(viewMinMip < AZ::RHI::ImageSubresourceRange::HighestSliceIndex)
        {
            uint32_t viewportStateMaxX = static_cast<uint32_t>(m_viewportState.m_maxX);
            uint32_t viewportStateMaxY = static_cast<uint32_t>(m_viewportState.m_maxY);
            m_viewportState.m_maxX = static_cast<float>(viewportStateMaxX >> viewMinMip);
            m_viewportState.m_maxY = static_cast<float>(viewportStateMaxY >> viewMinMip);
    
            m_scissorState.m_maxX = static_cast<uint32_t>(m_scissorState.m_maxX) >> viewMinMip;
            m_scissorState.m_maxY = static_cast<uint32_t>(m_scissorState.m_maxY) >> viewMinMip;
        }
        
        frameGraph.SetEstimatedItemCount(1);
    }
    
    void RenderJoyBillboardPass::CompileResources(const AZ::RHI::FrameGraphCompileContext& context)
    {
        if (m_shaderResourceGroup != nullptr)
        {
            if (m_shaderConstantsNeedUpdate)
            {
                if (!m_inputTextureIsAttachment && !m_inputTextureImage)
                {
                    auto renderJoySystem = RenderJoyInterface::Get();
                    m_inputTextureImage = renderJoySystem->GetInvalidParentPassTexture();
                }
                if (m_inputTextureImage)
                {
                    m_shaderResourceGroup->SetImage(m_inputTextureImageIndex, m_inputTextureImage);
                }
                
                m_shaderResourceGroup->SetConstant(m_isFlatscreenModeIndex, m_isFlatscreenMode);

                m_shaderResourceGroup->SetConstant(m_modelToWorldIndex, m_worldMatrix);
                m_shaderResourceGroup->SetConstant(m_alwaysFaceCameraIndex, m_alwaysFaceCamera);

                m_shaderResourceGroup->SetConstant(m_flatscreenLayoutIndex, m_flatscreenLayout);

            }

            BindPassSrg(context, m_shaderResourceGroup);
            m_shaderResourceGroup->Compile();
        }
    
        if (m_drawShaderResourceGroup != nullptr)
        {
            m_drawShaderResourceGroup->Compile();
            BindSrg(m_drawShaderResourceGroup->GetRHIShaderResourceGroup());
        }
    }
    
    void RenderJoyBillboardPass::BuildCommandListInternal(const AZ::RHI::FrameGraphExecuteContext& context)
    {
        AZ::RHI::CommandList* commandList = context.GetCommandList();
    
        SetSrgsForDraw(commandList);
    
        commandList->SetViewport(m_viewportState);
        commandList->SetScissor(m_scissorState);
    
        commandList->Submit(m_item);
    }

    void RenderJoyBillboardPass::SetWorldTransform(const AZ::Transform& worldTM)
    {
        m_worldMatrix = AZ::Matrix4x4::CreateFromTransform(worldTM);
        m_shaderConstantsNeedUpdate = true;
    }

    void RenderJoyBillboardPass::SetFlatscreenMode(float posX, float posY, float width, float height)
    {
        m_isFlatscreenMode = true;
        m_flatscreenLayout.Set(posX, posY, width, height);

        m_shaderConstantsNeedUpdate = true;
    }

    void RenderJoyBillboardPass::SetBillboardMode(bool alwaysFaceCamera)
    {
        m_isFlatscreenMode = false;
        m_alwaysFaceCamera = alwaysFaceCamera;
        m_shaderConstantsNeedUpdate = true;
    }

}   // namespace RenderJoy
