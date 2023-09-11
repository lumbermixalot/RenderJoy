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
#include <AzCore/StringFunc/StringFunc.h>

#include <Atom/RPI.Public/FeatureProcessorFactory.h>
#include <Atom/RPI.Public/Pass/PassSystemInterface.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Image/ImageSystemInterface.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>

#include <RenderJoy/RenderJoyPassBus.h>
#include <RenderJoy/RenderJoyTextureProviderBus.h>
#include <Render/RenderJoyBillboardPass.h>
#include <Render/RenderJoyShaderPass.h>
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
        auto renderJoyShaderPassClassName = AZ::Name(RenderJoyShaderPass::ClassNameStr);
        if (!passSystem->HasCreatorForClass(renderJoyShaderPassClassName))
        {
            passSystem->AddPassCreator(renderJoyShaderPassClassName, &RenderJoyShaderPass::Create);
        }

        RenderJoyRequestBus::Handler::BusConnect();
        AZ::RPI::FeatureProcessorFactory::Get()->RegisterFeatureProcessorWithInterface<RenderJoyFeatureProcessor, RenderJoyFeatureProcessorInterface>();
    }

    void RenderJoySystemComponent::Deactivate()
    {
        AZ::SystemTickBus::Handler::BusDisconnect();
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
        if (m_scenePtr->GetFeatureProcessor<RenderJoyFeatureProcessor>())
        {
            m_scenePtr->DisableFeatureProcessor<RenderJoyFeatureProcessor>();
            m_featureProcessor = nullptr;
        }

    }

    void RenderJoySystemComponent::CreateFeatureProcessor()
    {
        AZ_Assert(m_scenePtr != nullptr, "First define the scene!");
        m_featureProcessor = m_scenePtr->GetFeatureProcessor<RenderJoyFeatureProcessor>();
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

        m_entitiesToProcess.emplace(parentPassEntityId, passBusEntity);
        if (!AZ::SystemTickBus::Handler::BusIsConnected())
        {
            AZ::SystemTickBus::Handler::BusConnect();
        }

        //auto notifyTextureReadyFn = [parentPassEntityId = parentPassEntityId, passBusEntity = passBusEntity, this]()
        //    {
        //        auto passSystem = AZ::RPI::PassSystemInterface::Get();
        //        m_templatesFactory.CreateRenderJoyParentPassRequest(passSystem, parentPassEntityId, passBusEntity);
        //
        //        // Time to recreate the feature processor.
        //        // Later, when the feature processor is activated, it will query all the existing pass requests.
        //        CreateFeatureProcessor();
        //    };
        //AZ::TickBus::QueueFunction(AZStd::move(notifyTextureReadyFn));

        return true;
    }

    bool RenderJoySystemComponent::RemoveRenderJoyParentPass(AZ::EntityId parentPassEntityId)
    {
        // The feature processor needs to be destroyed before we can remove pass templates.
        DestroyFeatureProcessor();

        m_entitiesToProcess.emplace(parentPassEntityId, AZ::EntityId());
        if (!AZ::SystemTickBus::Handler::BusIsConnected())
        {
            AZ::SystemTickBus::Handler::BusConnect();
        }

        //// Defer template removal and recretation of the feature processor in the next cycle.
        //auto notifyTextureReadyFn = [parentPassEntityId = parentPassEntityId, this]()
        //    {
        //        auto passSystem = AZ::RPI::PassSystemInterface::Get();
        //        m_templatesFactory.RemoveTemplates(passSystem, parentPassEntityId);
        //
        //        if (m_shouldRecreateFeatureProcessor)
        //        {
        //            // Recreate the FP only if there are RenderJoy pass instances left.
        //            if (!m_templatesFactory.GetParentPassEntities().empty())
        //            {
        //                CreateFeatureProcessor();
        //            }
        //            else
        //            {
        //                m_scenePtr = nullptr;
        //            }
        //        }
        //    };
        //AZ::TickBus::QueueFunction(AZStd::move(notifyTextureReadyFn));

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

    AZ::Data::Instance<AZ::RPI::Image> RenderJoySystemComponent::GetDefaultInputTexture(uint32_t channelIndex) const
    {
        AZ::RPI::SystemImage imageEnumId = static_cast<AZ::RPI::SystemImage>(channelIndex);
        imageEnumId = (imageEnumId >= AZ::RPI::SystemImage::Count) ? AZ::RPI::SystemImage::Black : imageEnumId;
        return AZ::RPI::ImageSystemInterface::Get()->GetSystemImage(imageEnumId);
    }

    AZ::EntityId RenderJoySystemComponent::GetEntityIdFromPassName(const AZ::Name& passName) const
    {
        // All Pass names are unique, and in the form: "<Pass Class Name>_[%llu]" where %llu is a 64bit EntityId number
        static const AZStd::string EntityIdStartStr("_[");
        static const AZStd::string EntityIdEndStr("]");

        const AZStd::string passNameStr(passName.GetCStr());
        constexpr bool reverse = false;
        constexpr bool caseSensitive = true;
        size_t startPos = AZ::StringFunc::Find(passNameStr, EntityIdStartStr, /*size_t pos*/ 0, reverse, caseSensitive);
        if (startPos == AZStd::string::npos)
        {
            return AZ::EntityId();
        }
        startPos += EntityIdStartStr.size();
        size_t endPos = AZ::StringFunc::Find(passNameStr, EntityIdEndStr, startPos, reverse, caseSensitive);
        if (endPos == AZStd::string::npos)
        {
            return AZ::EntityId();
        }
        const AZStd::string entityIdStr = passNameStr.substr(startPos, endPos - startPos);
        char* end;
        const uint64_t uEntityId = strtoull( entityIdStr.c_str(), &end, 10);
        return AZ::EntityId(uEntityId);
    }

    // RenderJoyRequestBus interface implementation END
    ////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // SystemTickBus
    void RenderJoySystemComponent::OnSystemTick()
    {
        auto passSystem = AZ::RPI::PassSystemInterface::Get();

        if (m_entitiesToProcess.empty())
        {
            // If we have registered entities, it is time to stop looping and recreate the Feature processor.
            AZ::SystemTickBus::Handler::BusDisconnect();
            if (m_templatesFactory.GetParentPassCount())
            {
                CreateFeatureProcessor();
                return;
            }
        }

        // In this vector we'll store the data that we should remove from @m_entitiesToProcess.
        AZStd::vector<AZ::EntityId> removeList;

        // First we'll process passTemplate removals.
        for (auto const& [parentPassEntityId, passBusEntity] : m_entitiesToProcess)
        {
            // We always have to remove the parentPassEntityId templates first (even if later we are supposed
            // to add a new version).
            if (m_templatesFactory.EntityHasActivePasses(passSystem, parentPassEntityId))
            {
                // Try next tick.
                // The pass system is still in the process of destroying the feature processor.
                break;
            }
            else
            {
                m_templatesFactory.RemoveTemplates(passSystem, parentPassEntityId);
            }

            if (passBusEntity.IsValid())
            {
                m_templatesFactory.CreateRenderJoyParentPassRequest(passSystem, parentPassEntityId, passBusEntity);
            }

            removeList.push_back(parentPassEntityId);
        }

        for (const auto & entityId : removeList)
        {
            m_entitiesToProcess.erase(entityId);
        }

    }
    //////////////////////////////////////////////////////////////////////////

    namespace Utils
    {
        // Declared in RenderJoyPassBus.h
        bool IsRenderJoyPass(AZ::EntityId entityId)
        {
            if (!entityId.IsValid())
            {
                return false;
            }

            bool result = false;
            RenderJoyPassRequestBus::EnumerateHandlersId(entityId, [&](RenderJoyPassRequests* /*renderJoyPassRequest*/) -> bool
                {
                    result = true;
                    return true; // We expect only one handler anyways.
                });

            return result;
        }

        // Declared in RenderJoyTextureProviderBus.h
        bool IsRenderJoyTextureProvider(AZ::EntityId entityId)
        {
            if (!entityId.IsValid())
            {
                return false;
            }
        
            bool result = false;
            RenderJoyTextureProviderBus::EnumerateHandlersId(entityId, [&](RenderJoyTextureProvider * /*renderJoyPassRequest*/) -> bool {
                result = true;
                return true; // We expect only one handler anyways.
            });
        
            return result;
        }
    }

} // namespace RenderJoy
