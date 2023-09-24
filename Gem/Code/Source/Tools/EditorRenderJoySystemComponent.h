/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

//#include <AzCore/Component/Component.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/Entity/EditorEntityContextBus.h>

#include <Clients/RenderJoySystemComponent.h>

//#include <AzToolsFramework/Entity/EditorEntityContextBus.h>

namespace RenderJoy
{
    /// System component for RenderJoy editor
    class EditorRenderJoySystemComponent
        : public RenderJoySystemComponent
        , private AzToolsFramework::EditorEvents::Bus::Handler
        , private AzToolsFramework::EditorEntityContextNotificationBus::Handler
    {
        using BaseSystemComponent = RenderJoySystemComponent;
    public:
        //AZ_COMPONENT(EditorRenderJoySystemComponent, "{24185bbf-5ac7-4008-95ce-98b3af1df39f}");
        AZ_COMPONENT_DECL(EditorRenderJoySystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static constexpr char RenderJoyName[] = "RenderJoy";

        EditorRenderJoySystemComponent();
        ~EditorRenderJoySystemComponent() override;

        ////////////////////////////////////////////////////////////////////////
        // AztoolsFramework::EditorEvents::Bus::Handler overrides
        void NotifyRegisterViews() override;
        ////////////////////////////////////////////////////////////////////////

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;

        //////////////////////////////////////////////////////////////////////////////////////
        // AzToolsFramework::EditorEntityContextNotificationBus::Handler overrides START
        void OnStartPlayInEditorBegin() override;
        // AzToolsFramework::EditorEntityContextNotificationBus::Handler overrides END
        //////////////////////////////////////////////////////////////////////////////////////

    };
} // namespace RenderJoy
