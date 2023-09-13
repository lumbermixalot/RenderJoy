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

#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>

#include <RenderJoy/RenderJoyCommon.h>
#include <RenderJoy/RenderJoyTextureProviderBus.h>
#include <RenderJoy/RenderJoyFeatureProcessorInterface.h>

namespace RenderJoy
{
    class RenderJoyTextureComponentConfig final
        : public AZ::ComponentConfig
    {
    public:
        AZ_RTTI(RenderJoyTextureComponentConfig, "{87676040-B069-49DA-9C35-3FEA14FF2628}", ComponentConfig);
        AZ_CLASS_ALLOCATOR(RenderJoyTextureComponentConfig, AZ::SystemAllocator);
        static void Reflect(AZ::ReflectContext* context);

        RenderJoyTextureComponentConfig() = default;

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_imageAsset;
    };

    class RenderJoyTextureComponentController final
        : private AZ::Data::AssetBus::Handler
        , public RenderJoyTextureProviderBus::Handler
    {
    public:
        friend class EditorRenderJoyTextureComponent;

        AZ_RTTI(RenderJoyTextureComponentController, "{B52B0961-6A7C-464D-BA2D-3C4014174E63}");
        AZ_CLASS_ALLOCATOR(RenderJoyTextureComponentController, AZ::SystemAllocator);

        static void Reflect(AZ::ReflectContext* context);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        RenderJoyTextureComponentController() = default;
        RenderJoyTextureComponentController(const RenderJoyTextureComponentConfig& config);

        void Activate(AZ::EntityId entityId);
        void Deactivate();
        void SetConfiguration(const RenderJoyTextureComponentConfig& config);
        const RenderJoyTextureComponentConfig& GetConfiguration() const;

        //////////////////////////////////////////////////////////////////////////
        // RenderJoyTextureProviderBus overrides START
        bool IsImmutable() override { return true; }
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> GetStreamingImageAsset() const override;
        AZ::RHI::Format GetPixelFormat() const override;
        AZ::RHI::Size GetImageSize() const override;
        /// RenderJoyTextureProviderBus overrides END
        /////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // Data::AssetBus interface
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetError(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetCanceled(const AZ::Data::AssetId assetId) override;
        //////////////////////////////////////////////////////////////////////////

    private:

        AZ_DISABLE_COPY(RenderJoyTextureComponentController);

        static constexpr char LogName[] = "RenderJoyTextureComponentController";

        void OnConfigurationChanged();
        void OnImageAssetLoaded(AZ::Data::Asset<AZ::Data::AssetData> asset);

        AZ::EntityId m_entityId;
        
        RenderJoyTextureComponentConfig m_configuration;
        RenderJoyTextureComponentConfig m_prevConfiguration;
    };
} // namespace RenderJoy
