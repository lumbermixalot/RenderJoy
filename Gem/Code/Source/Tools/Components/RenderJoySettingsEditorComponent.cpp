/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include <AzCore/std/algorithm.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>

#include "RenderJoySettingsEditorComponent.h"

namespace RenderJoy
{
    void SettingsComponentConfig::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<SettingsComponentConfig, AZ::ComponentConfig>()
                ->Version(1)
                ->Field("RenderTargetWidth", &SettingsComponentConfig::m_renderTargetWith)
                ->Field("RenderTargetHeight", &SettingsComponentConfig::m_renderTargetHeight)
                ;
        }
    }

    void RenderJoySettingsEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        SettingsComponentConfig::Reflect(context);

        auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<RenderJoySettingsEditorComponent, EditorComponentBase>()
                ->Version(1)
                ->Field("SettingsConfig", &RenderJoySettingsEditorComponent::m_settingsConfig)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<RenderJoySettingsEditorComponent>(
                    "RenderJoy Settings", "General settings for RenderJoy")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "RenderJoy")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "editor/icons/components/viewport/component_placeholder.png")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZStd::vector<AZ::Crc32>({ AZ_CRC("Level", 0x9aeacc13) }))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoySettingsEditorComponent::m_settingsConfig, "Configuration", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &RenderJoySettingsEditorComponent::OnConfigChanged)
                    ;

                editContext->Class<SettingsComponentConfig>(
                    "SettingsComponentConfig", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SettingsComponentConfig::m_renderTargetWith, "Render Target Width", "")
                        ->Attribute(AZ::Edit::Attributes::Min, SettingsComponentConfig::MinDimensionSize)
                        ->Attribute(AZ::Edit::Attributes::Max, SettingsComponentConfig::MaxDimensionSize)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::ValuesOnly)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SettingsComponentConfig::m_renderTargetHeight, "Render Target Height", "")
                         ->Attribute(AZ::Edit::Attributes::Min, SettingsComponentConfig::MinDimensionSize)
                         ->Attribute(AZ::Edit::Attributes::Max, SettingsComponentConfig::MaxDimensionSize)
                         ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::ValuesOnly)
                    ;
            }
        }

    }

    void RenderJoySettingsEditorComponent::Activate()
    {
        ValidateConfig();
        RenderJoySettingsRequestBus::Handler::BusConnect();
    }

    void RenderJoySettingsEditorComponent::Deactivate()
    {
        RenderJoySettingsRequestBus::Handler::BusDisconnect();
    }

    //////////////////////////////////////////////////////////////////////////
    // RenderJoySettingsRequestBus overrides
    AZ::Vector2 RenderJoySettingsEditorComponent::GetRenderTargetSize() const
    {
        const float width = static_cast<float>(m_settingsConfig.m_renderTargetWith);
        const float height = static_cast<float>(m_settingsConfig.m_renderTargetHeight);
        return AZ::Vector2(width, height);
    }
    //////////////////////////////////////////////////////////////////////////

    void RenderJoySettingsEditorComponent::OnConfigChanged()
    {
        const float width = static_cast<float>(m_settingsConfig.m_renderTargetWith);
        const float height = static_cast<float>(m_settingsConfig.m_renderTargetHeight);
        ValidateConfig();
        RenderJoySettingsNotificationBus::Broadcast(
            &RenderJoySettingsNotifications::OnRenderTargetSizeChanged,
            AZ::Vector2(width, height));
    }

    void RenderJoySettingsEditorComponent::ValidateConfig()
    {

        m_settingsConfig.m_renderTargetWith = AZStd::max(SettingsComponentConfig::MinDimensionSize, m_settingsConfig.m_renderTargetWith);
        m_settingsConfig.m_renderTargetWith = AZStd::min(SettingsComponentConfig::MaxDimensionSize, m_settingsConfig.m_renderTargetWith);
        m_settingsConfig.m_renderTargetHeight = AZStd::max(SettingsComponentConfig::MinDimensionSize, m_settingsConfig.m_renderTargetHeight);
        m_settingsConfig.m_renderTargetHeight = AZStd::min(SettingsComponentConfig::MaxDimensionSize, m_settingsConfig.m_renderTargetHeight);
    }

    //////////////////////////////////////////////////////////////////////////

} // namespace RenderJoy
