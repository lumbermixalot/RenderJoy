/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/SerializeContext.h>

// #include <AzToolsFramework/API/ViewPaneOptions.h>
// #include <LyViewPaneNames.h>

#include "EditorRenderJoySystemComponent.h"
// #include "MainWindow.h"
// #include <RenderJoy/RenderJoySettingsBus.h>

#include <RenderJoy/RenderJoyTypeIds.h>

namespace RenderJoy
{
    AZ_COMPONENT_IMPL(EditorRenderJoySystemComponent, "EditorRenderJoySystemComponent",
        EditorRenderJoySystemComponentTypeId, BaseSystemComponent);

    void EditorRenderJoySystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorRenderJoySystemComponent, AZ::Component, BaseSystemComponent>()
                ->Version(0);
        }
    }

    EditorRenderJoySystemComponent::EditorRenderJoySystemComponent() = default;

    EditorRenderJoySystemComponent::~EditorRenderJoySystemComponent()
    {
        // AzToolsFramework::CloseViewPane(RenderJoyName);
        // AzToolsFramework::UnregisterViewPane(RenderJoyName);
    }

    void EditorRenderJoySystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("RenderJoySystemEditorService"));
    }

    void EditorRenderJoySystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("RenderJoySystemEditorService"));
    }

    void EditorRenderJoySystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void EditorRenderJoySystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void EditorRenderJoySystemComponent::Activate()
    {
        RenderJoySystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void EditorRenderJoySystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        RenderJoySystemComponent::Deactivate();
    }

    ////////////////////////////////////////////////////////////////////////
    // AztoolsFramework::EditorEvents::Bus::Handler overrides
    void EditorRenderJoySystemComponent::NotifyRegisterViews()
    {
        // AZ::Vector2 size(1280, 720);
        // RenderJoySettingsRequestBus::BroadcastResult(size, &RenderJoySettingsRequests::GetRenderTargetSize);
        // uint32_t width = aznumeric_cast<uint32_t>(AZStd::ceilf(size.GetX()));
        // uint32_t height = aznumeric_cast<uint32_t>(AZStd::ceilf(size.GetY()));
        // 
        // AzToolsFramework::ViewPaneOptions options;
        // options.paneRect = QRect(0, 0, width, height);
        // options.isDockable = false;
        // AzToolsFramework::RegisterViewPane<RenderJoy::MainWindow>(RenderJoyName, LyViewPane::CategoryOther, options);
    }
    ////////////////////////////////////////////////////////////////////////

} // namespace RenderJoy
