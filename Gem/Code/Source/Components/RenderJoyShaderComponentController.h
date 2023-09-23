/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TransformBus.h>

#include <Atom/RPI.Reflect/Shader/ShaderAsset.h>

#include <RenderJoy/RenderJoyCommon.h>
#include <RenderJoy/RenderJoyPassBus.h>
#include <RenderJoy/RenderJoyBus.h>
#include <RenderJoy/RenderJoyFeatureProcessorInterface.h>

namespace RenderJoy
{
    class RenderJoyShaderComponentConfig final
        : public AZ::ComponentConfig
    {
    public:
        AZ_RTTI(RenderJoyShaderComponentConfig, "{4DB526B0-449C-47E1-8919-F96E51D2093A}", ComponentConfig);
        AZ_CLASS_ALLOCATOR(RenderJoyShaderComponentConfig, AZ::SystemAllocator);
        static void Reflect(AZ::ReflectContext* context);

        RenderJoyShaderComponentConfig() = default;

        bool AreRenderTargetsEqual(const RenderJoyShaderComponentConfig& other) const;

        static constexpr uint32_t MinRenderTargetSize = 0; // 0 means use the same size as the main viewport.
        static constexpr uint32_t MaxRenderTargetSize = 7680; //We will allow 8K 7680x4320

        AZ::Data::Asset<AZ::RPI::ShaderAsset> m_shaderAsset;
        AZStd::array<AZ::EntityId, MaxInputChannels> m_inputChannels;
        uint32_t m_renderTargetWidth = 0; // Viewport width when 0
        uint32_t m_renderTargetHeight = 0; // Viewport height when 0
        AZ::RHI::Format m_outputFormat = AZ::RHI::Format::R32G32B32A32_FLOAT;
    };

    class RenderJoyShaderComponentController final
        : public RenderJoyPassRequestBus::Handler
        , private AZ::Data::AssetBus::Handler
        , private RenderJoyNotificationBus::Handler
    {
    public:
        friend class EditorRenderJoyShaderComponent;

        AZ_RTTI(RenderJoyShaderComponentController, "{74538D92-4682-4406-A924-5260EA0EA5D2}");
        AZ_CLASS_ALLOCATOR(RenderJoyShaderComponentController, AZ::SystemAllocator);

        static void Reflect(AZ::ReflectContext* context);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        RenderJoyShaderComponentController() = default;
        RenderJoyShaderComponentController(const RenderJoyShaderComponentConfig& config);

        void Activate(AZ::EntityId entityId);
        void Deactivate();
        void SetConfiguration(const RenderJoyShaderComponentConfig& config);
        const RenderJoyShaderComponentConfig& GetConfiguration() const;

        /////////////////////////////////////////////////////////////////
        /// RenderJoyPassRequestBus::Handler overrides START
        AZStd::vector<AZ::EntityId> GetEntitiesOnInputChannels() const override;
        AZ::Data::Asset<AZ::RPI::ShaderAsset> GetShaderAsset() const override;
        uint32_t GetRenderTargetWidth() const override;
        uint32_t GetRenderTargetHeight() const override;
        AZ::RHI::Format GetRenderTargetFormat() const override;
        /// RenderJoyPassRequestBus::Handler overrides END
        /////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////
        //! Data::AssetBus
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetError(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        /////////////////////////////////////////////////////////////////

    private:

        AZ_DISABLE_COPY(RenderJoyShaderComponentController);

        static constexpr char LogName[] = "RenderJoyShaderComponentController";

        void NotifyShaderAssetChanged(AZ::Data::Asset<AZ::Data::AssetData> asset);

        ///////////////////////////////////////////////////////////
        // RenderJoyNotificationBus::Handler overrides START
        void OnFeatureProcessorActivated() override;
        void OnFeatureProcessorDeactivated()override;
        // RenderJoyNotificationBus::Handler overrides END
        ///////////////////////////////////////////////////////////

        AZ::EntityId m_entityId;
        
        RenderJoyShaderComponentConfig m_configuration;
        RenderJoyShaderComponentConfig m_prevConfiguration;

        // If true, this component has an equivalent shader pass that is properly
        // configured and rendering.
        bool m_isRendering = false;

        void OnConfigurationChanged();

    };
} // namespace RenderJoy
