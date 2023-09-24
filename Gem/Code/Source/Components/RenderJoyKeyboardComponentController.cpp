/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <AzCore/Asset/AssetManager.h>
#include <AzCore/Asset/AssetManagerBus.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Serialization/SerializeContext.h>

#include <AzFramework/Entity/EntityContextBus.h>
#include <AzFramework/Entity/EntityContext.h>
#include <AzFramework/Scene/Scene.h>
#include <AzFramework/Scene/SceneSystemInterface.h>

#include <AzCore/RTTI/BehaviorContext.h>

#include <Atom/RPI.Public/Scene.h>

#include <RenderJoy/IKeyboardComponentsManager.h>
#include "RenderJoyKeyboardComponentController.h"

namespace RenderJoy
{
    void RenderJoyKeyboardComponentConfig::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyKeyboardComponentConfig>()
                ->Version(1)
                ->Field("ClearKeyPressedWaitTime", &RenderJoyKeyboardComponentConfig::m_maxWaitTimeToClearKeyPressedMilliseconds)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RenderJoyKeyboardComponentConfig>(
                    "RenderJoyKeyboardComponentConfig", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default
                        , &RenderJoyKeyboardComponentConfig::m_maxWaitTimeToClearKeyPressedMilliseconds
                        , "Clear Key Pressed Wait Time",
                        "Wait This Time In Milliseconds Before Clearing The Key Was Pressed State.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::ValuesOnly)
                    ;
            }
        }
    }

    void RenderJoyKeyboardComponentController::Reflect(AZ::ReflectContext* context)
    {
        RenderJoyKeyboardComponentConfig::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyKeyboardComponentController>()
                ->Version(0)
                ->Field("Configuration", &RenderJoyKeyboardComponentController::m_configuration);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RenderJoyKeyboardComponentController>(
                    "RenderJoyKeyboardComponentController", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyKeyboardComponentController::m_configuration, "Configuration", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ;
            }
        }
    }

    void RenderJoyKeyboardComponentController::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("TransformService"));
    }

    void RenderJoyKeyboardComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("RenderJoyKeyboardComponentService"));
    }

    void RenderJoyKeyboardComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("RenderJoyKeyboardComponentService"));
        incompatible.push_back(AZ_CRC_CE("RenderJoyShaderComponentService"));
        incompatible.push_back(AZ_CRC_CE("RenderJoyKeyboardComponentService"));
    }

    void RenderJoyKeyboardComponentController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("TransformService"));
    }

    RenderJoyKeyboardComponentController::RenderJoyKeyboardComponentController(const RenderJoyKeyboardComponentConfig& config)
        : m_configuration(config)
    {
    }

    void RenderJoyKeyboardComponentController::Activate(AZ::EntityId entityId)
    {
        m_prevConfiguration = m_configuration;
        m_entityId = entityId;
        RenderJoyKeyboardManagerInterface::Get()->RegisterKeyboardComponent(entityId);
        RenderJoyTextureProviderBus::Handler::BusConnect(entityId);
    }

    void RenderJoyKeyboardComponentController::Deactivate()
    {
        RenderJoyKeyboardManagerInterface::Get()->UnregisterKeyboardComponent(m_entityId);
        RenderJoyTextureProviderNotificationBus::Event(
            m_entityId, &RenderJoyTextureProviderNotification::OnImageChanged,
            AZ::Data::Instance<AZ::RPI::Image>());
        RenderJoyTextureProviderBus::Handler::BusDisconnect();
    }

    void RenderJoyKeyboardComponentController::SetConfiguration(const RenderJoyKeyboardComponentConfig& config)
    {
        m_configuration = config;
        m_prevConfiguration = config;
    }

    const RenderJoyKeyboardComponentConfig& RenderJoyKeyboardComponentController::GetConfiguration() const
    {
        return m_configuration;
    }

    void RenderJoyKeyboardComponentController::OnConfigurationChanged()
    {
        if (m_prevConfiguration.m_maxWaitTimeToClearKeyPressedMilliseconds != m_configuration.m_maxWaitTimeToClearKeyPressedMilliseconds)
        {
            RenderJoyKeyboardManagerInterface::Get()->UpdateClearKeyPressedMilliseconds(m_configuration.m_maxWaitTimeToClearKeyPressedMilliseconds);
        }

        m_prevConfiguration = m_configuration;
    }

    /////////////////////////////////////////////////////////////////
    /// RenderJoyTextureProviderBus::Handler overrides START
    AZ::Data::Instance<AZ::RPI::Image> RenderJoyKeyboardComponentController::GetImage() const
    {
        return RenderJoyKeyboardManagerInterface::Get()->GetKeyboardTexture();
    }
    /// RenderJoyTextureProviderBus::Handler overrides END
    /////////////////////////////////////////////////////////////////

}
