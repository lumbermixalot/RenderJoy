/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/SerializeContext.h>

#include <AzToolsFramework/API/ViewPaneOptions.h>
#include <LyViewPaneNames.h>

#include "RenderJoyEditorSystemComponent.h"
#include "MainWindow.h"
#include "PassEditor/RenderJoyPassEditorWindow.h"
#include <RenderJoy/RenderJoySettingsBus.h>


namespace RenderJoy
{
    void RenderJoyEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyEditorSystemComponent, AZ::Component>()
                ->Version(0);
        }
    }

    RenderJoyEditorSystemComponent::RenderJoyEditorSystemComponent() = default;

    RenderJoyEditorSystemComponent::~RenderJoyEditorSystemComponent()
    {
        AzToolsFramework::CloseViewPane(RenderJoyName);
        AzToolsFramework::UnregisterViewPane(RenderJoyName);
    }

    void RenderJoyEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("RenderJoyEditorService"));
    }

    void RenderJoyEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("RenderJoyEditorService"));
    }

    void RenderJoyEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void RenderJoyEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void RenderJoyEditorSystemComponent::Activate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void RenderJoyEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
    }

    ////////////////////////////////////////////////////////////////////////
    // AztoolsFramework::EditorEvents::Bus::Handler overrides
    void RenderJoyEditorSystemComponent::NotifyRegisterViews()
    {
        AZ::Vector2 size(1280, 720);
        RenderJoySettingsRequestBus::BroadcastResult(size, &RenderJoySettingsRequests::GetRenderTargetSize);
        uint32_t width = aznumeric_cast<uint32_t>(AZStd::ceilf(size.GetX()));
        uint32_t height = aznumeric_cast<uint32_t>(AZStd::ceilf(size.GetY()));

        AzToolsFramework::ViewPaneOptions options;
        options.paneRect = QRect(0, 0, width, height);
        options.isDockable = false;
        AzToolsFramework::RegisterViewPane<RenderJoy::MainWindow>(RenderJoyName, LyViewPane::CategoryOther, options);

        AzToolsFramework::RegisterViewPane<RenderJoy::RenderJoyPassEditorWindow>("RenderJoyPassEditor", LyViewPane::CategoryOther, options);
    }
    ////////////////////////////////////////////////////////////////////////

} // namespace RenderJoy
