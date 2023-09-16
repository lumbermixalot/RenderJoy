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
#include <Components/RenderJoyKeyboardComponent.h>

#include <RenderJoy/RenderJoyTypeIds.h>

namespace RenderJoy
{
    inline constexpr AZ::TypeId EditorRenderJoyKeyboardComponentTypeId { "{C08D1F9D-1F45-490A-BE14-4E5245FC0CD6}" };

    class EditorRenderJoyKeyboardComponent final
        : public AZ::Render::EditorRenderComponentAdapter<RenderJoyKeyboardComponentController, RenderJoyKeyboardComponent, RenderJoyKeyboardComponentConfig>
        , private AzToolsFramework::EditorComponentSelectionRequestsBus::Handler
        , private AzToolsFramework::EditorEntityInfoNotificationBus::Handler
    {
    public:
        using BaseClass = AZ::Render::EditorRenderComponentAdapter <RenderJoyKeyboardComponentController, RenderJoyKeyboardComponent, RenderJoyKeyboardComponentConfig>;
        AZ_EDITOR_COMPONENT(EditorRenderJoyKeyboardComponent, EditorRenderJoyKeyboardComponentTypeId, BaseClass);

        static void Reflect(AZ::ReflectContext* context);

        EditorRenderJoyKeyboardComponent();
        EditorRenderJoyKeyboardComponent(const RenderJoyKeyboardComponentConfig& config);

    private:

        // AZ::Component overrides
        void Activate() override;
        void Deactivate() override;

        // EditorComponentAdapter overrides;
        AZ::u32 OnConfigurationChanged() override;

        void OnEntityVisibilityChanged(bool visibility) override;

    };
}
