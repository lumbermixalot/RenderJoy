/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/
#pragma once

#include <AzCore/Memory/SystemAllocator.h>

#include <Atom/RHI/CommandList.h>
#include <Atom/RHI/DrawItem.h>
#include <Atom/RHI/ScopeProducer.h>

#include <Atom/RPI.Public/Pass/RenderPass.h>
#include <Atom/RPI.Public/Shader/Shader.h>
#include <Atom/RPI.Public/Shader/ShaderResourceGroup.h>
#include <Atom/RPI.Public/Shader/ShaderReloadNotificationBus.h>
#include <Atom/RPI.Public/PipelineState.h>

namespace RenderJoy
{
    //! A pass that renders to the entire screen using a single triangle.
    //! This pass is designed to work without any dedicated geometry buffers
    //! and instead issues a 3-vertex draw and relies on the vertex shader
    //! to generate a fullscreen triangle from the vertex ids.
    class RenderJoyBillboardPass
        : public AZ::RPI::RenderPass
        , public AZ::RPI::ShaderReloadNotificationBus::Handler
    {
        AZ_RPI_PASS(RenderJoyBillboardPass);
    
    public:
        AZ_RTTI(RenderJoyBillboardPass, "{BBCBCA8F-4E81-4970-92A5-CA1623901F6A}", AZ::RPI::RenderPass);
        AZ_CLASS_ALLOCATOR(RenderJoyBillboardPass, AZ::SystemAllocator);
        virtual ~RenderJoyBillboardPass();
    
        //! Creates a RenderJoyBillboardPass
        static AZ::RPI::Ptr<RenderJoyBillboardPass> Create(const AZ::RPI::PassDescriptor& descriptor);
    
        //! Return the shader
        AZ::Data::Instance<AZ::RPI::Shader> GetShader() const;
    
        //! Updates the shader options used in this pass
        void UpdateShaderOptions(const AZ::RPI::ShaderOptionList& shaderOptions);
    
    protected:
        RenderJoyBillboardPass(const AZ::RPI::PassDescriptor& descriptor);
    
        // Pass behavior overrides...
        void InitializeInternal() override;
        void FrameBeginInternal(FramePrepareParams params) override;
    
        // Scope producer functions...
        void SetupFrameGraphDependencies(AZ::RHI::FrameGraphInterface frameGraph) override;
        void CompileResources(const AZ::RHI::FrameGraphCompileContext& context) override;
        void BuildCommandListInternal(const AZ::RHI::FrameGraphExecuteContext& context) override;
    
        // ShaderReloadNotificationBus overrides...
        void OnShaderReinitialized(const AZ::RPI::Shader& shader) override;
        void OnShaderAssetReinitialized(const AZ::Data::Asset<AZ::RPI::ShaderAsset>& shaderAsset) override;
        void OnShaderVariantReinitialized(const AZ::RPI::ShaderVariant& shaderVariant) override;
    
        AZ::RHI::Viewport m_viewportState;
        AZ::RHI::Scissor m_scissorState;
    
        // The fullscreen shader that will be used by the pass
        AZ::Data::Instance<AZ::RPI::Shader> m_shader;
    
        // Manages changes to the pipeline state
        AZ::RPI::PipelineStateForDraw m_pipelineStateForDraw;
    
        // The draw item submitted by this pass
        AZ::RHI::DrawItem m_item;
    
        // The stencil reference value for the draw item
        uint32_t m_stencilRef;
    
        AZ::Data::Instance<AZ::RPI::ShaderResourceGroup> m_drawShaderResourceGroup;
    
    private:
        void LoadShader();
        void UpdateSrgs();
        void BuildDrawItem();
    
        AZ::RPI::PassDescriptor m_passDescriptor;
    };
}   // namespace RenderJoy
