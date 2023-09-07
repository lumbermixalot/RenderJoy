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

#include <RenderJoy/RenderJoyFeatureProcessorInterface.h>
#include <Components/RenderJoyBillboardComponentController.h>

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
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyBillboardComponentConfig::m_alwaysFaceCamera, "Should this billboard always face the camera?", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyBillboardComponentConfig::m_shaderEntityId, "Entity with a RenderJoy shader", "")
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

        AZ::TransformNotificationBus::Handler::BusConnect(m_entityId);

        if (!m_configuration.m_shaderEntityId.IsValid())
        {
            // nothing to do at the moment.
            return;
        }

        // Ask the RenderJoy system if a pass template can be created.
        auto renderJoySystem = RenderJoyInterface::Get();
        AZ_Assert(!!renderJoySystem, "Failed to find the RenderJoy system interface");
        auto outcome = renderJoySystem->CreateRenderJoyPassTemplate(m_configuration.m_shaderEntityId);
        if (!outcome.IsSuccess())
        {
            // Given that we can not create a proper set of Render Joy passes, let's create the RenderJoy Invalid pipeline
            // to give a visual clue to the user. We will create a dummy pipeline. This will force the recreation of the RenderJoy Feature processor.
            renderJoySystem->AddInvalidRendeJoyPipeline(m_entityId, m_configuration.m_shaderEntityId);
            return;
        }

        // We got a valid RenderJoy pipeline, let's recreate the feature processor and enjoy the view!
        AZ::RPI::PassTemplate newRenderJoyTemplate(outcome.TakeValue());
        renderJoySystem->AddRenderJoyPipeline(m_entityId, m_configuration.m_shaderEntityId, newRenderJoyTemplate);

        //m_featureProcessor = AZ::RPI::Scene::GetFeatureProcessorForEntity<RenderJoyFeatureProcessorInterface>(entityId);



        //RenderJoyBus::EventResult(shaderAsset, currentPassEntity, &RenderJoyPassRequests::GetShaderAsset);

        //AZ_Assert(m_featureProcessor, "RenderJoyBillboardComponentController was unable to find a RenderJoyFeatureProcessor on the EntityContext provided.");

    }

    void RenderJoyBillboardComponentController::Deactivate()
    {
        AZ::TransformNotificationBus::Handler::BusDisconnect();
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

        auto featureProcessor = AZ::RPI::Scene::GetFeatureProcessorForEntity<RenderJoyFeatureProcessorInterface>(m_entityId);
        featureProcessor->UpdateWorldTransform(m_entityId, world);

    }

    void RenderJoyBillboardComponentController::OnConfigurationChanged()
    {
        //AZ_Printf(LogName, "%s\n", __FUNCTION_);
    }
}
