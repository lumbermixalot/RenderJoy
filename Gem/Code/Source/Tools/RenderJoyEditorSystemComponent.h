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

#include <Clients/RenderJoySystemComponent.h>

//#include <AzToolsFramework/Entity/EditorEntityContextBus.h>

namespace RenderJoy
{
    /// System component for RenderJoy editor
    class RenderJoyEditorSystemComponent
        : public RenderJoySystemComponent
        , private AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = RenderJoySystemComponent;
    public:
        //AZ_COMPONENT(RenderJoyEditorSystemComponent, "{24185bbf-5ac7-4008-95ce-98b3af1df39f}");
        AZ_COMPONENT_DECL(RenderJoyEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static constexpr char RenderJoyName[] = "RenderJoy";

        RenderJoyEditorSystemComponent();
        ~RenderJoyEditorSystemComponent() override;

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
    };
} // namespace RenderJoy
