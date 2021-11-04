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

#include <RenderJoy/RenderJoySettingsBus.h> 

namespace RenderJoy
{
    class SettingsComponentConfig final
        : public AZ::ComponentConfig
    {
    public:
        AZ_RTTI(SettingsComponentConfig, "{785EF4FB-3B00-4FC7-980C-A887F2BAE5B4}", AZ::ComponentConfig);

        static void Reflect(AZ::ReflectContext* context);

        static constexpr uint32_t MinDimensionSize = 240;
        static constexpr uint32_t MaxDimensionSize = 7680; //We will allow 8K 7680x4320

        uint32_t m_renderTargetWith = 1280;
        uint32_t m_renderTargetHeight = 720;
    };

    /*
    * This class is designed to be used under automation, but the user can add it to an entity
    * and manually specify a json file with a list of asset paths to load asynchronously. From an user point of view
    * it has no value, but for debugging it can be useful to try the AssetCollectionAsyncLoader API without having to write
    * a test suite for it.
    */
    class RenderJoySettingsEditorComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , public RenderJoySettingsRequestBus::Handler
        , AZ::Data::AssetBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(RenderJoySettingsEditorComponent, "{25F9A8B8-9870-4F57-8716-C428970670C2}"
            , AzToolsFramework::Components::EditorComponentBase);

        static constexpr char LogName[] = " RenderJoySettingsEditorComponent";

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& services)
        {
            services.push_back(AZ::Crc32("RenderJoySettingsComponentService"));
        }
        static void GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& services)
        {
            services.push_back(AZ::Crc32("RenderJoySettingsComponentService"));
        }
        static void GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& services) {}
        static void GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& services) {}

        //////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // RenderJoySettingsRequestBus overrides
        AZ::Vector2 GetRenderTargetSize() const override;
        //////////////////////////////////////////////////////////////////////////

        void OnConfigChanged();

    protected:
        SettingsComponentConfig m_settingsConfig;

        void ValidateConfig();

    };
} // namespace RenderJoy
