/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include "RenderJoySystemComponent.h"

#include <RenderJoy/RenderJoyTypeIds.h>

#include <AzCore/Serialization/SerializeContext.h>

#include <Atom/RPI.Public/FeatureProcessorFactory.h>
#include <Atom/RPI.Public/Pass/PassSystemInterface.h>
#include <Atom/RPI.Public/Scene.h>

#include <Render/RenderJoyFeatureProcessor.h>

namespace RenderJoy
{
    AZ_COMPONENT_IMPL(RenderJoySystemComponent, "RenderJoySystemComponent",
        RenderJoySystemComponentTypeId);

    void RenderJoySystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoySystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }

        RenderJoyFeatureProcessor::Reflect(context);
    }

    void RenderJoySystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("RenderJoySystemService"));
    }

    void RenderJoySystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("RenderJoySystemService"));
    }

    void RenderJoySystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("RPISystem"));
    }

    void RenderJoySystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    RenderJoySystemComponent::RenderJoySystemComponent()
    {
        if (RenderJoyInterface::Get() == nullptr)
        {
            RenderJoyInterface::Register(this);
        }
    }

    RenderJoySystemComponent::~RenderJoySystemComponent()
    {
        if (RenderJoyInterface::Get() == this)
        {
            RenderJoyInterface::Unregister(this);
        }
    }

    void RenderJoySystemComponent::Init()
    {
    }

    void RenderJoySystemComponent::Activate()
    {
        RenderJoyRequestBus::Handler::BusConnect();
        AZ::RPI::FeatureProcessorFactory::Get()->RegisterFeatureProcessor<RenderJoyFeatureProcessor>();
    }

    void RenderJoySystemComponent::Deactivate()
    {
        AZ::RPI::FeatureProcessorFactory::Get()->UnregisterFeatureProcessor<RenderJoyFeatureProcessor>();
        RenderJoyRequestBus::Handler::BusDisconnect();

        auto passSystem = AZ::RPI::PassSystemInterface::Get();
        m_templatesFactory.RemoveAllTemplates(passSystem);
    }

    void RenderJoySystemComponent::DestroyFeatureProcessor()
    {
        if (!m_scenePtr || !m_featureProcessor)
        {
            return;
        }
        m_scenePtr->DisableFeatureProcessor<RenderJoyFeatureProcessorInterface>();
        m_featureProcessor = nullptr;
    }

    void RenderJoySystemComponent::CreateFeatureProcessor()
    {
        AZ_Assert(m_scenePtr != nullptr, "First define the scene!");
        AZ_Assert(m_featureProcessor == nullptr, "Can not create if still exists!");
        m_featureProcessor = m_scenePtr->EnableFeatureProcessor<RenderJoyFeatureProcessorInterface>();
    }

    ////////////////////////////////////////////////////////////////////////
    // RenderJoyRequestBus interface implementation START
    PassTemplateOutcome RenderJoySystemComponent::CreateRenderJoyPassTemplate(AZ::EntityId passBusEntity) const
    {
        //AZ::RPI::PassTemplate passTemplate;
        //return AZ::Success(AZStd::move(passTemplate));
        return AZ::Failure(AZStd::string::format("CreateRenderJoyPassTemplate failed for entity=%s", passBusEntity.ToString().c_str()));
    }

    bool RenderJoySystemComponent::AddInvalidRenderJoyPipeline(AZ::EntityId pipelineEntityId, AZ::EntityId passBusEntity)
    {
        if (m_passInstances.contains(pipelineEntityId))
        {
            AZ_Error(LogName, false, "The RenderJoy pipeline for entity=%s already exists!", pipelineEntityId.ToString().c_str());
            return false;
        }

        if (!m_scenePtr)
        {
            m_scenePtr = AZ::RPI::Scene::GetSceneForEntityId(pipelineEntityId);
            AZ_Assert(m_scenePtr != nullptr, "No scene for entity=%s", pipelineEntityId.ToString().c_str());
        }

        auto passSystem = AZ::RPI::PassSystemInterface::Get();
        auto passTemplate = m_templatesFactory.CreateInvalidRenderJoyParentPassTemplate(passSystem, { "Whatever" });
        m_passInstances[pipelineEntityId] = passTemplate.m_name;

        // Time to recreate the feature processor.
        DestroyFeatureProcessor();
        CreateFeatureProcessor();

        return true;
    }

    bool RenderJoySystemComponent::AddRenderJoyPipeline(AZ::EntityId pipelineEntityId, AZ::EntityId passBusEntity, const AZ::RPI::PassTemplate& passTemplate)
    {
        AZ_Printf(LogName, "%s pipelineEntityId=%s, passBusEntity=%s, passTemplate=%s\n", __FUNCTION__,
            pipelineEntityId.ToString().c_str(), passBusEntity.ToString().c_str(), passTemplate.m_name.GetCStr());
        return false;
    }

    void RenderJoySystemComponent::RemoveRenderJoyPipeline(AZ::EntityId pipelineEntityId)
    {
        auto itor = m_passInstances.find(pipelineEntityId);
        if (itor == m_passInstances.end())
        {
            AZ_Warning(LogName, false, "The RenderJoy pipeline for entity=%s does not exist", pipelineEntityId.ToString().c_str());
            return;
        }

        auto templateName = itor->second;
        auto passSystem = AZ::RPI::PassSystemInterface::Get();
        m_templatesFactory.RemoveTemplate(passSystem, templateName);
        m_passInstances.erase(itor);

        DestroyFeatureProcessor();

        // Recreate the FP only if there are RenderJoy pass instances left.
        if (!m_passInstances.empty())
        {
            CreateFeatureProcessor();
        }
        else
        {
            m_scenePtr = nullptr;
        }

        AZ_Printf(LogName, "%s pipelineEntityId=%s\n", __FUNCTION__, pipelineEntityId.ToString().c_str());
    }
    // RenderJoyRequestBus interface implementation END
    ////////////////////////////////////////////////////////////////////////

} // namespace RenderJoy
