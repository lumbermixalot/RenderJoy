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
    class RenderJoyKeyboardComponentConfig final
        : public AZ::ComponentConfig
    {
    public:
        AZ_RTTI(RenderJoyKeyboardComponentConfig, "{EB0A05C1-8E90-49C3-884E-FA1A4460F003}", ComponentConfig);
        AZ_CLASS_ALLOCATOR(RenderJoyKeyboardComponentConfig, AZ::SystemAllocator);
        static void Reflect(AZ::ReflectContext* context);

        RenderJoyKeyboardComponentConfig() = default;

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_imageAsset;
    };

    class RenderJoyKeyboardComponentController final
        : private AZ::Data::AssetBus::Handler
        , public RenderJoyTextureProviderBus::Handler
    {
    public:
        friend class EditorRenderJoyKeyboardComponent;

        AZ_RTTI(RenderJoyKeyboardComponentController, "{AE372E55-7FD4-4BB3-8F59-C29557A5A1AC}");
        AZ_CLASS_ALLOCATOR(RenderJoyKeyboardComponentController, AZ::SystemAllocator);

        static void Reflect(AZ::ReflectContext* context);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        RenderJoyKeyboardComponentController() = default;
        RenderJoyKeyboardComponentController(const RenderJoyKeyboardComponentConfig& config);

        void Activate(AZ::EntityId entityId);
        void Deactivate();
        void SetConfiguration(const RenderJoyKeyboardComponentConfig& config);
        const RenderJoyKeyboardComponentConfig& GetConfiguration() const;

        //////////////////////////////////////////////////////////////////////////
        // RenderJoyTextureProviderBus overrides START
        bool IsImmutable() override { return false; }
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

        AZ_DISABLE_COPY(RenderJoyKeyboardComponentController);

        static constexpr char LogName[] = "RenderJoyKeyboardComponentController";

        void OnConfigurationChanged();
        void OnImageAssetLoaded(AZ::Data::Asset<AZ::Data::AssetData> asset);

        AZ::EntityId m_entityId;
        
        RenderJoyKeyboardComponentConfig m_configuration;
        RenderJoyKeyboardComponentConfig m_prevConfiguration;
    };
} // namespace RenderJoy
