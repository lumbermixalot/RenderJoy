/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <Atom/Feature/Utils/EditorRenderComponentAdapter.h>

#include <AzToolsFramework/API/ComponentEntitySelectionBus.h>
#include <AzToolsFramework/Entity/EditorEntityInfoBus.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentAdapter.h>
#include <Components/RenderJoyShaderComponent.h>

#include <RenderJoy/RenderJoyTypeIds.h>
#include <RenderJoy/RenderJoyBus.h>

namespace RenderJoy
{
    class RenderJoyShaderPass;

    inline constexpr AZ::TypeId EditorRenderJoyShaderComponentTypeId { "{D6CAD35B-00A1-4E33-894F-0F8BD4DD682A}" };

    class EditorRenderJoyShaderComponent final
        : public AZ::Render::EditorRenderComponentAdapter<RenderJoyShaderComponentController, RenderJoyShaderComponent, RenderJoyShaderComponentConfig>
        , private AzToolsFramework::EditorComponentSelectionRequestsBus::Handler
        , private AzToolsFramework::EditorEntityInfoNotificationBus::Handler
    {
    public:
        using BaseClass = AZ::Render::EditorRenderComponentAdapter <RenderJoyShaderComponentController, RenderJoyShaderComponent, RenderJoyShaderComponentConfig>;
        AZ_EDITOR_COMPONENT(EditorRenderJoyShaderComponent, EditorRenderJoyShaderComponentTypeId, BaseClass);

        static void Reflect(AZ::ReflectContext* context);

        EditorRenderJoyShaderComponent();
        EditorRenderJoyShaderComponent(const RenderJoyShaderComponentConfig& config);

        static constexpr char LogName[] = "EditorRenderJoyShaderComponent";

    private:

        // AZ::Component overrides
        void Activate() override;
        void Deactivate() override;

        // EditorComponentAdapter overrides;
        AZ::u32 OnConfigurationChanged() override;

        // Captures the current rendered image and saves it to disk.
        AZ::u32 OnSaveToDisk();
        // Helper function that makes the SaveToDisk button disabled or enabled.
        bool IsSaveToDiskDisabled();

        // ///////////////////////////////////////////////////////////
        // // RenderJoyNotificationBus::Handler overrides START
        // void OnFeatureProcessorActivated() override;
        // void OnFeatureProcessorDeactivated() override;
        // // RenderJoyNotificationBus::Handler overrides END
        // ///////////////////////////////////////////////////////////

        void OnEntityVisibilityChanged(bool visibility) override;

        // We use this pass only to make render target capture requests.
        // Will get the real reference once we get the OnFeatureProcessorActivated notification. 
        RenderJoyShaderPass* m_shaderPass = nullptr;

    };
}
