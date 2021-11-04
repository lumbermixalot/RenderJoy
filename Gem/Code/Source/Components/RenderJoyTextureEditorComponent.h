/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Component/Component.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>

#include <RenderJoy/RenderJoyTextureProviderBus.h> 

namespace RenderJoy
{
    class TextureComponentConfig final
        : public AZ::ComponentConfig
    {
    public:
        AZ_RTTI(TextureComponentConfig, "{87676040-B069-49DA-9C35-3FEA14FF2628}", AZ::ComponentConfig);

        static void Reflect(AZ::ReflectContext* context);

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_imageAsset;

    };

    /*
    * Provides a RGBA immutable texture usable as input channel for a RenderJoy shader.
    */
    class RenderJoyTextureEditorComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , public RenderJoyTextureProviderBus::Handler
        , public AZ::Data::AssetBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(RenderJoyTextureEditorComponent, "{B52B0961-6A7C-464D-BA2D-3C4014174E63}"
            , AzToolsFramework::Components::EditorComponentBase);

        static constexpr char LogName[] = " RenderJoyTextureEditorComponent";

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& services)
        {
            services.push_back(AZ::Crc32("RenderJoyTextureComponentService"));
        }

        static void GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& services)
        {
            services.push_back(AZ::Crc32("RenderJoyTextureComponentService"));
            services.push_back(AZ::Crc32("RenderJoyPassComponentService"));
            services.push_back(AZ::Crc32("RenderJoyKeyboardComponentService"));
        }

        static void GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& services) {}
        static void GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& services) {}

        //////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // RenderJoyTextureProviderBus overrides
        bool IsImmutable() override { return true; }
        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> GetStreamingImageAsset() const override;
        AZ::RHI::Format GetPixelFormat() const override;
        AZ::RHI::Size GetImageSize() const override;
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // Data::AssetBus interface
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetError(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetCanceled(const AZ::Data::AssetId assetId) override;
        //////////////////////////////////////////////////////////////////////////

        void OnConfigChanged();

    protected:
        void StartLoadingImage(AZ::Data::AssetId assetId);
        void OnImageAssetLoaded(AZ::Data::Asset<AZ::Data::AssetData> asset);

        TextureComponentConfig m_config;
    };
} // namespace RenderJoy
