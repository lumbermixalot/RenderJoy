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
#include <Atom/RPI.Reflect/Shader/ShaderAsset.h>

#include <RenderJoy/RenderJoyCommon.h>
#include <RenderJoy/RenderJoyPassBus.h>


namespace RenderJoy
{
    class PassComponentConfig final
        : public AZ::ComponentConfig
    {
    public:
        AZ_RTTI(PassComponentConfig, "{4DB526B0-449C-47E1-8919-F96E51D2093A}", AZ::ComponentConfig);

        static void Reflect(AZ::ReflectContext* context);

        AZ::Data::Asset<AZ::RPI::ShaderAsset> m_shaderAsset;
        AZStd::array<AZ::EntityId, MaxInputChannels> m_inputChannels;
        bool m_isOutputPass = false;
    };

    /*
    * This class is designed to be used under automation, but the user can add it to an entity
    * and manually specify a json file with a list of asset paths to load asynchronously. From an user point of view
    * it has no value, but for debugging it can be useful to try the AssetCollectionAsyncLoader API without having to write
    * a test suite for it.
    */
    class RenderJoyPassEditorComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , AZ::Data::AssetBus::Handler
        , RenderJoyPassRequestBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(RenderJoyPassEditorComponent, "{D6CAD35B-00A1-4E33-894F-0F8BD4DD682A}"
            , AzToolsFramework::Components::EditorComponentBase);

        static constexpr char LogName[] = " RenderJoyPassEditorComponent";

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& services)
        {
            services.push_back(AZ::Crc32("RenderJoyPassComponentService"));
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

        void OnConfigChanged();

    protected:
        //////////////////////////////////////////////////////////////////////////
        // Data::AssetBus interface
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetError(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        //////////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////////
        // RenderJoyPassRequestBus interface
        bool IsOutputPass() override;
        AZStd::vector<AZ::EntityId> GetEntitiesOnInputChannels() override;
        AZ::Data::Asset<AZ::RPI::ShaderAsset> GetShaderAsset() override { return m_config.m_shaderAsset; }
        //////////////////////////////////////////////////////////////////////////

        PassComponentConfig m_config;
    };
} // namespace RenderJoy
