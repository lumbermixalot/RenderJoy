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

#include <Render/RenderJoyBillboardPass.h>
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
        auto passSystem = AZ::RPI::PassSystemInterface::Get();
        auto renderJoyBillboardPassClassName = AZ::Name(RenderJoyBillboardPass::PassNameStr);
        if (!passSystem->HasCreatorForClass(renderJoyBillboardPassClassName))
        {
            passSystem->AddPassCreator(renderJoyBillboardPassClassName, &RenderJoyBillboardPass::Create);
        }

        RenderJoyRequestBus::Handler::BusConnect();
        AZ::RPI::FeatureProcessorFactory::Get()->RegisterFeatureProcessorWithInterface<RenderJoyFeatureProcessor, RenderJoyFeatureProcessorInterface>();
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
        if (!m_scenePtr)
        {
            return;
        }
        m_scenePtr->DisableFeatureProcessor<RenderJoyFeatureProcessor>();
        m_featureProcessor = nullptr;
    }

    void RenderJoySystemComponent::CreateFeatureProcessor()
    {
        AZ_Assert(m_scenePtr != nullptr, "First define the scene!");
        AZ_Assert(m_featureProcessor == nullptr, "Can not create if still exists!");
        m_featureProcessor = m_scenePtr->EnableFeatureProcessor<RenderJoyFeatureProcessor>();
    }

    ////////////////////////////////////////////////////////////////////////
    // RenderJoyRequestBus interface implementation START
    bool RenderJoySystemComponent::AddRenderJoyParentPass(AZ::EntityId parentPassEntityId, AZ::EntityId passBusEntity)
    {
        if (!m_scenePtr)
        {
            m_scenePtr = AZ::RPI::Scene::GetSceneForEntityId(parentPassEntityId);
            AZ_Error(LogName, m_scenePtr != nullptr, "The RenderJoy pipeline for entity=%s already exists!", parentPassEntityId.ToString().c_str());
            AZ_Assert(m_scenePtr != nullptr, "No scene for entity=%s", parentPassEntityId.ToString().c_str());
            if (!m_scenePtr)
            {
                return false;
            }
        }

        auto passSystem = AZ::RPI::PassSystemInterface::Get();
        m_templatesFactory.CreateRenderJoyParentPassRequest(passSystem, parentPassEntityId, passBusEntity);

        // Time to recreate the feature processor.
        // Later, when the feature processor is activated, it will query all the existing pass requests.
        DestroyFeatureProcessor();
        CreateFeatureProcessor();

        return true;
    }

    bool RenderJoySystemComponent::RemoveRenderJoyParentPass(AZ::EntityId parentPassEntityId)
    {
        auto passSystem = AZ::RPI::PassSystemInterface::Get();
        m_templatesFactory.RemoveTemplates(passSystem, parentPassEntityId);

        DestroyFeatureProcessor();

        // Recreate the FP only if there are RenderJoy pass instances left.
        if (!m_templatesFactory.GetParentPassEntities().empty())
        {
            CreateFeatureProcessor();
        }
        else
        {
            m_scenePtr = nullptr;
        }

        //AZ_Printf(LogName, "%s parentPassEntityId=%s\n", __FUNCTION__, parentPassEntityId.ToString().c_str());
        return true;
    }

    AZStd::vector<AZ::EntityId> RenderJoySystemComponent::GetParentPassEntities() const
    {
        return m_templatesFactory.GetParentPassEntities();
    }

    AZStd::shared_ptr<AZ::RPI::PassRequest> RenderJoySystemComponent::GetPassRequest(AZ::EntityId parentPassEntityId) const
    {
        return m_templatesFactory.GetParentPassRequest(parentPassEntityId);
    }

    AZ::Name RenderJoySystemComponent::GetBillboardPassName(AZ::EntityId parentPassEntityId) const
    {
        return m_templatesFactory.GetBillboardPassName(parentPassEntityId);
    }
    // RenderJoyRequestBus interface implementation END
    ////////////////////////////////////////////////////////////////////////

} // namespace RenderJoy
