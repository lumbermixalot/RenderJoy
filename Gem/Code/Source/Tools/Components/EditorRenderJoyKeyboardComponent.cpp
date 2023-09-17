/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Tools/Components/EditorRenderJoyKeyboardComponent.h>
#include <AzFramework/StringFunc/StringFunc.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/Entity/EditorEntityInfoBus.h>
#include <AzToolsFramework/API/EditorAssetSystemAPI.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/IO/SystemFile.h>

namespace RenderJoy
{
    void EditorRenderJoyKeyboardComponent::Reflect(AZ::ReflectContext* context)
    {
        BaseClass::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorRenderJoyKeyboardComponent, BaseClass>()
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<EditorRenderJoyKeyboardComponent>(
                    "RenderJoy Keyboard", "Provides a texture that encodes the state of the keyboard.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Graphics/RenderJoy")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Icons/Components/Viewport/Component_Placeholder.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->Attribute(AZ::Edit::Attributes::HelpPageURL, "")
                    ;
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->ConstantProperty("EditorRenderJoyKeyboardComponentTypeId", BehaviorConstant(AZ::Uuid(EditorRenderJoyKeyboardComponentTypeId)))
                ->Attribute(AZ::Script::Attributes::Module, "render")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Automation);
        }
    }

    EditorRenderJoyKeyboardComponent::EditorRenderJoyKeyboardComponent()
    {
    }

    EditorRenderJoyKeyboardComponent::EditorRenderJoyKeyboardComponent(const RenderJoyKeyboardComponentConfig& config)
        : BaseClass(config)
    {
    }

    void EditorRenderJoyKeyboardComponent::Activate()
    {
        BaseClass::Activate();
        AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusConnect(GetEntityId());
        AzToolsFramework::EditorEntityInfoNotificationBus::Handler::BusConnect();
    }

    void EditorRenderJoyKeyboardComponent::Deactivate()
    {
        AzToolsFramework::EditorEntityInfoNotificationBus::Handler::BusDisconnect();
        AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusDisconnect();
        BaseClass::Deactivate();
    }


    void EditorRenderJoyKeyboardComponent::OnEntityVisibilityChanged([[maybe_unused]] bool visibility)
    {

    }

    // EditorComponentAdapter overrides
    AZ::u32 EditorRenderJoyKeyboardComponent::OnConfigurationChanged()
    {
        m_controller.OnConfigurationChanged();
        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

} // namespace RenderJoy
