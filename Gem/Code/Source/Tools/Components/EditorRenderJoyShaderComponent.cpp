/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Tools/Components/EditorRenderJoyShaderComponent.h>
#include <AzFramework/StringFunc/StringFunc.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/Entity/EditorEntityInfoBus.h>
#include <AzToolsFramework/API/EditorAssetSystemAPI.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/IO/SystemFile.h>

namespace RenderJoy
{
    void EditorRenderJoyShaderComponent::Reflect(AZ::ReflectContext* context)
    {
        BaseClass::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorRenderJoyShaderComponent, BaseClass>()
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<EditorRenderJoyShaderComponent>(
                    "RenderJoy Shader", "The RenderJoy Shader component")
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
            behaviorContext->ConstantProperty(EditorRenderJoySystemComponentTypeId, BehaviorConstant(AZ::Uuid(EditorRenderJoySystemComponentTypeId)))
                ->Attribute(AZ::Script::Attributes::Module, "render")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Automation);
        }
    }

    EditorRenderJoyShaderComponent::EditorRenderJoyShaderComponent()
    {
    }

    EditorRenderJoyShaderComponent::EditorRenderJoyShaderComponent(const RenderJoyShaderComponentConfig& config)
        : BaseClass(config)
    {
    }

    void EditorRenderJoyShaderComponent::Activate()
    {
        BaseClass::Activate();
        AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusConnect(GetEntityId());
        AzToolsFramework::EditorEntityInfoNotificationBus::Handler::BusConnect();

        //AZ::u64 entityId = (AZ::u64)GetEntityId();
        //m_controller.m_configuration.m_entityId = entityId;
    }

    void EditorRenderJoyShaderComponent::Deactivate()
    {
        AzToolsFramework::EditorEntityInfoNotificationBus::Handler::BusDisconnect();
        AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusDisconnect();
        BaseClass::Deactivate();
    }


    void EditorRenderJoyShaderComponent::OnEntityVisibilityChanged(bool visibility)
    {
        if (visibility)
        {
            //m_controller.EnableFeatureProcessor(GetEntityId());
        }
        else
        {
            //m_controller.DisableFeatureProcessor();
        }
    }

    // EditorComponentAdapter overrides
    AZ::u32 EditorRenderJoyShaderComponent::OnConfigurationChanged()
    {
        m_controller.OnConfigurationChanged();
        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

} // namespace RenderJoy
