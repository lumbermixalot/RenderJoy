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

#include "RenderJoyEditorSystemComponent.h"
// #include "MainWindow.h"
// #include <RenderJoy/RenderJoySettingsBus.h>

#include <RenderJoy/RenderJoyTypeIds.h>

namespace RenderJoy
{
    AZ_COMPONENT_IMPL(RenderJoyEditorSystemComponent, "RenderJoyEditorSystemComponent",
        RenderJoyEditorSystemComponentTypeId, BaseSystemComponent);

    void RenderJoyEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyEditorSystemComponent, AZ::Component, BaseSystemComponent>()
                ->Version(0);
        }
    }

    RenderJoyEditorSystemComponent::RenderJoyEditorSystemComponent() = default;

    RenderJoyEditorSystemComponent::~RenderJoyEditorSystemComponent()
    {
        // AzToolsFramework::CloseViewPane(RenderJoyName);
        // AzToolsFramework::UnregisterViewPane(RenderJoyName);
    }

    void RenderJoyEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("RenderJoySystemEditorService"));
    }

    void RenderJoyEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("RenderJoySystemEditorService"));
    }

    void RenderJoyEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void RenderJoyEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void RenderJoyEditorSystemComponent::Activate()
    {
        RenderJoySystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void RenderJoyEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        RenderJoySystemComponent::Deactivate();
    }

    ////////////////////////////////////////////////////////////////////////
    // AztoolsFramework::EditorEvents::Bus::Handler overrides
    void RenderJoyEditorSystemComponent::NotifyRegisterViews()
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
