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
#include <Atom/RPI.Public/Image/StreamingImage.h>

#include <RenderJoy/RenderJoyPassBus.h>
#include <RenderJoy/RenderJoyTextureProviderBus.h>

namespace RenderJoy
{
    class IRenderJoySrgDataProvider;

    //! A pass that renders to the entire image attachment using a single triangle.
    //! This pass is designed to work without any dedicated geometry buffers
    //! and instead issues a 3-vertex draw and relies on the vertex shader
    //! to generate a fullscreen triangle from the vertex ids.
    class RenderJoyShaderPass final
        : public AZ::RPI::RenderPass
        , public AZ::RPI::ShaderReloadNotificationBus::Handler
        //, public RenderJoyPassNotificationBus::Handler
        , public RenderJoyTextureProviderNotificationBus::MultiHandler
    {
        AZ_RPI_PASS(RenderJoyShaderPass);

    public:
        static constexpr uint32_t InvalidInputChannelIndex = static_cast<uint32_t>(-1);
        static constexpr char ClassNameStr[] = "RenderJoyShaderPass";

        AZ_RTTI(RenderJoyShaderPass, "{37F1C0D0-6168-410F-AA70-04154A937545}", AZ::RPI::RenderPass);
        AZ_CLASS_ALLOCATOR(RenderJoyShaderPass, AZ::SystemAllocator, 0);
        virtual ~RenderJoyShaderPass();

        //! Creates a RenderJoyShaderPass
        static AZ::RPI::Ptr<RenderJoyShaderPass> Create(const AZ::RPI::PassDescriptor& descriptor);

        // Clears the initialized flag so that the next time PrepareFrameInternal is called, it will update the pipeline state
        void Invalidate();

        //AZ::RPI::Pass Overrides
        bool IsEnabled() const override
        {
            return true;
        }

        static constexpr size_t ImageChannelsCount = 4;

    private:
        RenderJoyShaderPass(const AZ::RPI::PassDescriptor& descriptor);
        void Init();

        // Pass behavior overrides...
        void FrameBeginInternal(AZ::RPI::Pass::FramePrepareParams params) override;
        void BuildInternal() override;

        bool m_initialized = false;
        AZ::RHI::Viewport m_viewportState;
        AZ::RHI::Scissor m_scissorState;

        // The fullscreen shader that will be used by the pass
        AZ::Data::Instance<AZ::RPI::Shader> m_shader;

        // The draw item submitted by this pass
        AZ::RHI::DrawItem m_item;

        // The stencil reference value for the draw item
        uint8_t m_stencilRef;

        // Scope producer functions...
        void SetupFrameGraphDependencies(AZ::RHI::FrameGraphInterface frameGraph) override;
        void CompileResources(const AZ::RHI::FrameGraphCompileContext& context) override;

        // RenderPass functions...
        void BuildCommandListInternal(const AZ::RHI::FrameGraphExecuteContext& context) override;

        ///////////////////////////////////////////////////////////////////
        // ShaderReloadNotificationBus overrides...
        void OnShaderReinitialized(const AZ::RPI::Shader& shader) override;
        void OnShaderAssetReinitialized(const AZ::Data::Asset<AZ::RPI::ShaderAsset>& shaderAsset) override;
        void OnShaderVariantReinitialized(const AZ::RPI::ShaderVariant& shaderVariant) override;
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////
        // RenderJoyPassNotificationBus overrides...
        // void OnShaderAssetChanged(AZ::Data::Asset<AZ::RPI::ShaderAsset> newShaderAsset) override;
        // void OnInputChannelEntityChanged(uint32_t inputChannelIndex, AZ::EntityId newEntityId) override;
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////
        // RenderJoyTextureProviderNotificationBus overrides...
        void OnStreamingImageAssetChanged(AZ::Data::Asset<AZ::RPI::StreamingImageAsset> imageAsset) override;
        void OnPixelBufferChanged(const void* pixels, AZ::RHI::Size imageSize, uint32_t bytesPerRow) override;
        ///////////////////////////////////////////////////////////////////

        void LoadShader();
        void CacheRenderJoySrgConstantsIndices();
        bool SetImageAssetForChannel(uint32_t channelIndex, AZ::Data::Asset<AZ::RPI::StreamingImageAsset> imageAsset);
        bool SetMutableImageForChannel(AZ::EntityId entityId, uint32_t channelIndex);
        void SetDefaultImageForChannel(uint32_t imageChannelIdx);
        void CreateImageForChannel(uint32_t imageChannelIdx, const AZStd::string& imageName, size_t width, size_t height, uint32_t color);
        void RemoveImageForChannel(uint32_t imageChannelIdx);
        void SetupCopyImageItem(const AZ::RHI::FrameGraphCompileContext& context);
        void UpdatePixelDataForChannel(uint32_t channelIndex, const void* pixels, const AZ::RHI::Size& imageSize, uint32_t bytesPerRow);


        // Helper function.
        uint32_t GetInputChannelIndexFromEntityId(const AZ::EntityId& entityId);

        AZ::EntityId m_entityId;
        AZStd::array<AZ::EntityId, ImageChannelsCount> m_entitiesOnInputChannels;

        AZ::RPI::PassDescriptor m_passDescriptor;
        IRenderJoySrgDataProvider* m_renderJoySrgDataProvider = nullptr;

        // At runtime each image in this array can be immutable (AZ::RPI::StreamingImage)
        // or mutable (AZ::RPI::AttachmentImage). The image comes from a RenderJoyTextureProviderBus::Handler.
        // - If immutable, the pixel content doesn't change, but the image reference can change
        //   and we will get notified via RenderJoyTextureProviderNotificationBus with the new
        //   streamingimageasset, and the whole image reference will be replace.
        // - If mutable, we can expect periodic updates from RenderJoyTextureProviderNotificationBus,
        //   and for each of those updates we will upload data from the CPU to the GPU.
        //   a Typical scenario would be the RenderJoyTextureProvider is the keyboard
        //   "texture" which encodes key down/up states and each time the user presses
        //   or releases a key the AZ::RPI::AttachmentImage will be updated from CPU data.
        AZStd::array<AZ::Data::Instance<AZ::RPI::Image>, ImageChannelsCount> m_imageChannels;
        AZStd::array<AZ::Vector4, ImageChannelsCount> m_imageChannelResolutions;

        // In cases where the user chose the output of this pass to be one
        // of the input channels, this persistent attachment will be used for such "recursive"
        // situation. For the first frame this attachment will start as color black.
        // For the second frame we'll copy the output target into this attachment.
        // In other words for Frame N, the content of this attachment will be
        // this pass output target for Frame N-1.
        AZ::Data::Instance<AZ::RPI::AttachmentImage> m_prevFrameOutputAsInput;
        uint32_t m_inputChannelIndexForPrevFrameOutputAsInput = InvalidInputChannelIndex;
        AZ::RHI::CopyItem m_currentOutputCopyItem;

        // The RenderJoySrg constants
        AZ::RHI::ShaderInputConstantIndex m_resolutionIndex;
        AZ::RHI::ShaderInputConstantIndex m_timeIndex;
        AZ::RHI::ShaderInputConstantIndex m_timeDeltaIndex;
        AZ::RHI::ShaderInputConstantIndex m_frameIndex;
        AZ::RHI::ShaderInputConstantIndex m_frameRateIndex;
        AZ::RHI::ShaderInputConstantIndex m_mouseIndex;
        AZ::RHI::ShaderInputConstantIndex m_imageChannelResolutionsIndex;
        AZ::RHI::ShaderInputImageIndex m_imageChannelsIndex;

    };
}   // namespace RenderJoy
