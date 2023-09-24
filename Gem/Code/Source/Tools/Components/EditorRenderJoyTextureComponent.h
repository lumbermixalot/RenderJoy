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
#include <Components/RenderJoyTextureComponent.h>

#include <RenderJoy/RenderJoyTypeIds.h>

namespace RenderJoy
{
    inline constexpr AZ::TypeId EditorRenderJoyTextureComponentTypeId { "{BE391ED7-FDC8-41DB-9F29-CE2DAE1B8B84}" };

    class EditorRenderJoyTextureComponent final
        : public AZ::Render::EditorRenderComponentAdapter<RenderJoyTextureComponentController, RenderJoyTextureComponent, RenderJoyTextureComponentConfig>
        , private AzToolsFramework::EditorComponentSelectionRequestsBus::Handler
        , private AzToolsFramework::EditorEntityInfoNotificationBus::Handler
    {
    public:
        using BaseClass = AZ::Render::EditorRenderComponentAdapter <RenderJoyTextureComponentController, RenderJoyTextureComponent, RenderJoyTextureComponentConfig>;
        AZ_EDITOR_COMPONENT(EditorRenderJoyTextureComponent, EditorRenderJoyTextureComponentTypeId, BaseClass);

        static void Reflect(AZ::ReflectContext* context);

        EditorRenderJoyTextureComponent();
        EditorRenderJoyTextureComponent(const RenderJoyTextureComponentConfig& config);

    private:

        // AZ::Component overrides
        void Activate() override;
        void Deactivate() override;

        // EditorComponentAdapter overrides;
        AZ::u32 OnConfigurationChanged() override;

        void OnEntityVisibilityChanged(bool visibility) override;

    };
}
