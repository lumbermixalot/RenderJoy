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
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>

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
        required.push_back(AZ_CRC_CE("AssetCatalogService"));
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
        if (!m_featureProcessor)
        {
            m_featureProcessor = m_scenePtr->EnableFeatureProcessor<RenderJoyFeatureProcessor>();
        }
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

        // We can not remove pass templates while the Feature Processor is still active.
        DestroyFeatureProcessor();

        auto notifyTextureReadyFn = [parentPassEntityId = parentPassEntityId, passBusEntity = passBusEntity, this]()
            {
                auto passSystem = AZ::RPI::PassSystemInterface::Get();
                m_templatesFactory.CreateRenderJoyParentPassRequest(passSystem, parentPassEntityId, passBusEntity);

                // Time to recreate the feature processor.
                // Later, when the feature processor is activated, it will query all the existing pass requests.
                CreateFeatureProcessor();
            };
        AZ::TickBus::QueueFunction(AZStd::move(notifyTextureReadyFn));

        return true;
    }

    bool RenderJoySystemComponent::RemoveRenderJoyParentPass(AZ::EntityId parentPassEntityId)
    {
        // The feature processor needs to be destroyed before we can remove pass templates.
        DestroyFeatureProcessor();

        // Defer template removal and recretation of the feature processor in the next cycle.
        auto notifyTextureReadyFn = [parentPassEntityId = parentPassEntityId, this]()
            {
                auto passSystem = AZ::RPI::PassSystemInterface::Get();
                m_templatesFactory.RemoveTemplates(passSystem, parentPassEntityId);

                // Recreate the FP only if there are RenderJoy pass instances left.
                if (!m_templatesFactory.GetParentPassEntities().empty())
                {
                    CreateFeatureProcessor();
                }
                else
                {
                    m_scenePtr = nullptr;
                }
            };
        AZ::TickBus::QueueFunction(AZStd::move(notifyTextureReadyFn));


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

    AZ::Data::Instance<AZ::RPI::Image> RenderJoySystemComponent::GetInvalidParentPassTexture() const
    {
        if (!m_invalidParentPassTexture && !m_asyncLoadStarted)
        {
            m_asyncLoadStarted = true;
            auto textureReadCB = [this](AZ::Data::Asset<AZ::Data::AssetData> asset, bool success)
                {
                    if (success)
                    {
                        AZ_Printf(LogName, "Successfully loaded the Invalid ParentPass Texture!");
                        m_invalidParentPassTexture = AZ::RPI::StreamingImage::FindOrCreate(asset);
                    }
                    else
                    {
                        AZ_Error(LogName, false, "Failed to load the Invalid ParentPass Texture!");
                    }
                };
            m_asyncAssetLoader = AZ::RPI::AssetUtils::AsyncAssetLoader::Create<AZ::RPI::StreamingImageAsset>(AZStd::string(InvalidPipelineTexturePath), 0, textureReadCB);
            return nullptr;

        }
        return m_invalidParentPassTexture;
    }
    // RenderJoyRequestBus interface implementation END
    ////////////////////////////////////////////////////////////////////////

} // namespace RenderJoy
