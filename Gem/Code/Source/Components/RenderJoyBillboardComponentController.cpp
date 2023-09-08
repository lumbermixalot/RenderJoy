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
#include <AzCore/RTTI/BehaviorContext.h>

#include <AzFramework/Entity/EntityContextBus.h>
#include <AzFramework/Entity/EntityContext.h>
#include <AzFramework/Scene/Scene.h>
#include <AzFramework/Scene/SceneSystemInterface.h>

#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Pass/PassFilter.h>

#include <Render/RenderJoyBillboardPass.h>
//#include <RenderJoy/RenderJoyFeatureProcessorInterface.h>
#include "RenderJoyBillboardComponentController.h"

namespace RenderJoy
{
    void RenderJoyBillboardComponentConfig::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyBillboardComponentConfig>()
                ->Version(1)
                ->Field("AlwaysFaceCamera", &RenderJoyBillboardComponentConfig::m_alwaysFaceCamera)
                ->Field("ShaderEntityId", &RenderJoyBillboardComponentConfig::m_shaderEntityId)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RenderJoyBillboardComponentConfig>(
                    "RenderJoyBillboardComponentConfig", "Configuration data for the RenderJoy Billboard Component.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyBillboardComponentConfig::m_alwaysFaceCamera, "Always Face Camera", "Should this billboard always face the camera?")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyBillboardComponentConfig::m_shaderEntityId, "RenderJoy Shader", "Entity with a RenderJoy shader")
                    ;
            }
        }
    }

    void RenderJoyBillboardComponentController::Reflect(AZ::ReflectContext* context)
    {
        RenderJoyBillboardComponentConfig::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyBillboardComponentController>()
                ->Version(0)
                ->Field("Configuration", &RenderJoyBillboardComponentController::m_configuration);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RenderJoyBillboardComponentController>(
                    "RenderJoyBillboardComponentController", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyBillboardComponentController::m_configuration, "Configuration", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ;
            }
        }
    }

    void RenderJoyBillboardComponentController::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("TransformService"));
    }

    void RenderJoyBillboardComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("RenderJoyBillboardService"));
    }

    void RenderJoyBillboardComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("RenderJoyBillboardService"));
    }

    void RenderJoyBillboardComponentController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("TransformService"));
    }

    RenderJoyBillboardComponentController::RenderJoyBillboardComponentController(const RenderJoyBillboardComponentConfig& config)
        : m_configuration(config)
    {
    }

    void RenderJoyBillboardComponentController::Activate(AZ::EntityId entityId)
    {
        m_entityId = entityId;

        RenderJoyNotificationBus::Handler::BusConnect();
        AZ::TransformNotificationBus::Handler::BusConnect(m_entityId);

        if (!m_configuration.m_shaderEntityId.IsValid())
        {
            // nothing to do at the moment.
            return;
        }

        // Ask the RenderJoy system if a pass template can be created.
        auto renderJoySystem = RenderJoyInterface::Get();
        AZ_Assert(!!renderJoySystem, "Failed to find the RenderJoy system interface");
        const auto success = renderJoySystem->AddRenderJoyParentPass(m_entityId, m_configuration.m_shaderEntityId);
        AZ_Error(LogName, success, "Failed to add RenderJoy Pipeline.");

    }

    void RenderJoyBillboardComponentController::Deactivate()
    {
        AZ::TransformNotificationBus::Handler::BusDisconnect();
        RenderJoyNotificationBus::Handler::BusDisconnect();
        
        auto renderJoySystem = RenderJoyInterface::Get();
        AZ_Assert(!!renderJoySystem, "Failed to find the RenderJoy system interface");
        renderJoySystem->RemoveRenderJoyParentPass(m_entityId);
    }

    void RenderJoyBillboardComponentController::SetConfiguration(const RenderJoyBillboardComponentConfig& config)
    {
        m_configuration = config;
        m_prevConfiguration = config;
    }

    const RenderJoyBillboardComponentConfig& RenderJoyBillboardComponentController::GetConfiguration() const
    {
        return m_configuration;
    }

    void RenderJoyBillboardComponentController::OnTransformChanged([[maybe_unused]] const AZ::Transform& local, [[maybe_unused]] const AZ::Transform& world)
    {
        // if (!m_featureProcessor)
        // {
        //     return;
        // }

        //auto featureProcessor = AZ::RPI::Scene::GetFeatureProcessorForEntity<RenderJoyFeatureProcessorInterface>(m_entityId);
        if (m_billboardPass)
        {
            m_billboardPass->SetWorldTransform(world);
        }

    }

    void RenderJoyBillboardComponentController::OnConfigurationChanged()
    {
        if (m_prevConfiguration.m_shaderEntityId != m_configuration.m_shaderEntityId)
        {

            auto renderJoySystem = RenderJoyInterface::Get();
            AZ_Assert(!!renderJoySystem, "Failed to find the RenderJoy system interface");
            // Because the Feature Processor will be recreated, we need to invalidate
            // our reference to the Billboard pass.
            m_billboardPass = nullptr;
            renderJoySystem->AddRenderJoyParentPass(m_entityId, m_configuration.m_shaderEntityId);
        }
        else if (m_prevConfiguration.m_alwaysFaceCamera != m_configuration.m_alwaysFaceCamera)
        {
            if (m_billboardPass)
            {
                m_billboardPass->SetAlwaysFaceCamera(m_configuration.m_alwaysFaceCamera);
            }
        }

        m_prevConfiguration = m_configuration;
    }

    ///////////////////////////////////////////////////////////
    // RenderJoyNotificationBus::Handler overrides START
    void RenderJoyBillboardComponentController::OnFeatureProcessorActivated()
    {
        // auto scenePtr = AZ::RPI::Scene::GetSceneForEntityId(m_entityId);
        // // Keep a reference to the billboard pass.
        // auto renderJoySystem = RenderJoyInterface::Get();
        // auto passName = renderJoySystem->GetBillboardPassName(m_entityId);
        // AZ::RPI::PassFilter passFilter = AZ::RPI::PassFilter::CreateWithPassName(passName, scenePtr);
        // AZ::RPI::Pass* existingPass = AZ::RPI::PassSystemInterface::Get()->FindFirstPass(passFilter);
        // m_billboardPass = azrtti_cast<RenderJoyBillboardPass*>(existingPass);
        // AZ_Error(LogName, m_billboardPass != nullptr, "%s Failed to find RenderJoyBillboardPass as: %s", __FUNCTION__, passName.GetCStr());
        // AZ_Assert(m_billboardPass != nullptr, "%s Failed to find RenderJoyBillboardPass as: %s", __FUNCTION__, passName.GetCStr());
        // 
        // // Update shader constant data.
        // AZ::Transform transform = AZ::Transform::CreateIdentity();
        // AZ::TransformBus::EventResult(transform, m_entityId, &AZ::TransformBus::Events::GetWorldTM);
        // m_billboardPass->SetWorldTransform(transform);
        // m_billboardPass->SetAlwaysFaceCamera(m_configuration.m_alwaysFaceCamera);
    }

    void RenderJoyBillboardComponentController::OnFeatureProcessorDeactivated()
    {
        // clear the reference to the billboard pass.
        m_billboardPass = nullptr;
    }
    // RenderJoyNotificationBus::Handler overrides END
    ///////////////////////////////////////////////////////////
}
