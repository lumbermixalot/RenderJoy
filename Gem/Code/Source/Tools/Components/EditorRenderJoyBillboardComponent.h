/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Atom/Feature/Utils/EditorRenderComponentAdapter.h>

#include <AzCore/Component/TickBus.h>
#include <AzFramework/Entity/EntityDebugDisplayBus.h>
#include <AzToolsFramework/API/ComponentEntitySelectionBus.h>
#include <AzToolsFramework/Entity/EditorEntityInfoBus.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentAdapter.h>
#include <Components/RenderJoyBillboardComponent.h>

#include <RenderJoy/RenderJoyTypeIds.h>

namespace RenderJoy
{
    inline constexpr AZ::TypeId EditorComponentTypeId { "{50A91A4A-234F-4475-A164-3A8FD0FE8FBE}" };

    class EditorRenderJoyBillboardComponent final
        : public AZ::Render::EditorRenderComponentAdapter<RenderJoyBillboardComponentController, RenderJoyBillboardComponent, RenderJoyBillboardComponentConfig>
        , private AzToolsFramework::EditorComponentSelectionRequestsBus::Handler
        , private AzFramework::EntityDebugDisplayEventBus::Handler
        , private AZ::TickBus::Handler
        , private AzToolsFramework::EditorEntityInfoNotificationBus::Handler
    {
    public:
        using BaseClass = AZ::Render::EditorRenderComponentAdapter <RenderJoyBillboardComponentController, RenderJoyBillboardComponent, RenderJoyBillboardComponentConfig>;
        AZ_EDITOR_COMPONENT(EditorRenderJoyBillboardComponent, EditorComponentTypeId, BaseClass);

        static void Reflect(AZ::ReflectContext* context);

        EditorRenderJoyBillboardComponent();
        EditorRenderJoyBillboardComponent(const RenderJoyBillboardComponentConfig& config);

        // AZ::Component overrides
        void Activate() override;
        void Deactivate() override;

    protected:

        void OnEntityVisibilityChanged(bool visibility) override;

    private:

        // AZ::TickBus overrides
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;


    };
}
