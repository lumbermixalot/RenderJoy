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

#include <AzFramework/Input/Channels/InputChannel.h>
#include <AzFramework/Input/Devices/InputDevice.h>
#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
//#include <AzFramework/Input/Buses/Requests/InputSystemCursorRequestBus.h>

#include <Atom/RPI.Public/FeatureProcessorFactory.h>
#include <Atom/RPI.Public/Pass/PassSystemInterface.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
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
        RenderJoyKeyboardManagerInterface::Register(this);
        RenderJoyInterface::Register(this);
        RenderJoySrgDataProviderInterface::Register(this);
    }

    RenderJoySystemComponent::~RenderJoySystemComponent()
    {
        RenderJoyKeyboardManagerInterface::Unregister(this);
        RenderJoyInterface::Unregister(this);
        RenderJoySrgDataProviderInterface::Unregister(this);
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

        RenderJoyNotificationBus::Handler::BusConnect();
        RenderJoyRequestBus::Handler::BusConnect();
        AZ::RPI::FeatureProcessorFactory::Get()->RegisterFeatureProcessorWithInterface<RenderJoyFeatureProcessor, RenderJoyFeatureProcessorInterface>();
        AzFramework::InputChannelNotificationBus::Handler::BusConnect();
    }

    void RenderJoySystemComponent::Deactivate()
    {
        AzFramework::InputChannelNotificationBus::Handler::BusDisconnect();
        RenderJoyNotificationBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
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
            {
                // piggy back and keep fresh values of window size.
                auto viewportContextInterface = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
                auto viewportContext = viewportContextInterface->GetViewportContextByScene(m_scenePtr);
                m_viewportSize = viewportContext->GetViewportSize();
            }
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

    AZ::Name RenderJoySystemComponent::GetShaderPassName(AZ::EntityId shaderPassEntityId) const
    {
        return m_templatesFactory.GetShaderPassName(shaderPassEntityId);
    }

    AZ::Data::Instance<AZ::RPI::Image> RenderJoySystemComponent::GetInvalidParentPassTexture() const
    {
        if (!m_invalidParentPassTexture && !m_asyncLoadStarted)
        {
            m_asyncLoadStarted = true;
            auto textureReadCB = [this](AZ::Data::Asset<AZ::Data::AssetData> asset)
                {
                    if (asset.IsReady())
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

    void RenderJoySystemComponent::RecreateAllPasses(AZ::EntityId shaderPassTriggerEntityId)
    {
        if (!shaderPassTriggerEntityId.IsValid() || !m_templatesFactory.GetParentPassCount() || !m_scenePtr)
        {
            return;
        }

        AZ::EntityId parentPassEntityId;
        AZ::EntityId passBusEntityId;
        if (!m_templatesFactory.FindParentAndOutputPassEntity(shaderPassTriggerEntityId
            , parentPassEntityId, passBusEntityId))
        {
            AZ_Warning(LogName, false, "Failed to find parent entity and output entity for shader pass entity %s.\n", shaderPassTriggerEntityId.ToString().c_str());
            return;
        }

        AZ_Printf(LogName, "The RendeJoy Passes will be recreated upon request from %s.\n", shaderPassTriggerEntityId.ToString().c_str());

        AddRenderJoyParentPass(parentPassEntityId, passBusEntityId);

        // // We can not remove pass templates while the Feature Processor is still active.
        // DestroyFeatureProcessor();
        // 
        // if (!AZ::SystemTickBus::Handler::BusIsConnected())
        // {
        //     AZ::SystemTickBus::Handler::BusConnect();
        // }
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

    //////////////////////////////////////////////////////////////////////////
    // TickBus
    void RenderJoySystemComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
    {
        m_frameCounter++;
        m_runTime += deltaTime;
        m_frameTimeDelta = deltaTime;
        m_oneSecondMark += deltaTime;
        if (m_oneSecondMark >= 1.0f)
        {
            m_oneSecondMark = 0.0f;
            m_fps = static_cast<float>(m_frameCounter - m_frameCounterStamp);
            m_frameCounterStamp = m_frameCounter;
        }
        // This is how ShaderToy keeps the click state enabled for a certain amount of time,
        // and we replicate the behavior here.
        if ( m_isLeftButtonClick && (m_runTime > (m_timeSinceLastClick + 1.0f)) )
        {
            m_isLeftButtonClick = false;
        }

        if (m_keyboardTextureMgr)
        {
            m_keyboardTextureMgr->OnTick(deltaTime, time);
        }

    }
    //////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////
    // IRenderJoySrgDataProvider interface implementation START
    float RenderJoySystemComponent::GetTime()
    {
        return m_runTime;
    }

    float RenderJoySystemComponent::GetTimeDelta()
    {
        return m_frameTimeDelta;
    }
    
    int RenderJoySystemComponent::GetFramesCount()
    {
        return m_frameCounter;
    }
    
    float RenderJoySystemComponent::GetFramesPerSecond()
    {
        return m_fps;
    }

    void RenderJoySystemComponent::GetMouseData(AZ::Vector2& currentPos, AZ::Vector2& clickPos, bool& isLeftButtonDown, bool& isLeftButtonClick)
    {
        currentPos = m_currentMousePos;
        clickPos = m_clickMousePos;
        isLeftButtonDown = m_isLeftButtonDown;
        isLeftButtonClick = m_isLeftButtonClick;
    }

    void RenderJoySystemComponent::ResetFrameCounter(int newValue)
    {
        m_oneSecondMark = 0.0;
        m_runTime = 0.0;
        m_frameCounter = newValue;
        m_frameCounterStamp = newValue;
    }
    // IRenderJoySrgDataProvider interface implementation END
    ////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////
    // RenderJoyNotificationBus::Handler overrides START
    void RenderJoySystemComponent::OnFeatureProcessorActivated()
    {
        if (!AZ::TickBus::Handler::BusIsConnected())
        {
            ResetFrameCounter(0);
            AZ::TickBus::Handler::BusConnect();
        }
    }

    void RenderJoySystemComponent::OnFeatureProcessorDeactivated()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }
    // RenderJoyNotificationBus::Handler overrides END
    ///////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////
    // InputChannelNotificationBus::Handler overrides START
    void RenderJoySystemComponent::OnInputChannelEvent(const AzFramework::InputChannel& inputChannel, [[maybe_unused]] bool& hasBeenConsumed)
    {
        //AZ_Printf(LogName, "Got channel event from %s.\n", inputChannel.GetInputChannelId().GetName());
        const auto& inputDeviceId = inputChannel.GetInputDevice().GetInputDeviceId();
        if (AzFramework::InputDeviceMouse::IsMouseDevice(inputDeviceId))
        {
            OnMouseChannelEvent(inputChannel, hasBeenConsumed);
        }
        else if (AzFramework::InputDeviceKeyboard::IsKeyboardDevice(inputDeviceId))
        {
            OnKeyboardChannelEvent(inputChannel, hasBeenConsumed);
        }
    }
    // InputChannelNotificationBus::Handler overrides END
    ///////////////////////////////////////////////////////////

    void RenderJoySystemComponent::OnMouseChannelEvent(const AzFramework::InputChannel& inputChannel, [[maybe_unused]] bool& hasBeenConsumed)
    {
        if (inputChannel.GetInputChannelId() == AzFramework::InputDeviceMouse::Button::Left)
        {
            //hasBeenConsumed = true;
            const bool isClick = inputChannel.IsStateBegan();
            m_isLeftButtonDown = isClick || inputChannel.IsStateUpdated();
            if (isClick)
            {
                // We can only set this to false after one second, per ShaderToy's semantics.
                m_isLeftButtonClick = true;
                m_timeSinceLastClick = m_runTime;
                m_clickMousePos = m_currentMousePos;
            }
            else if (inputChannel.IsStateEnded())
            {
                m_isLeftButtonDown = false;
                m_isLeftButtonClick = false;
            }
        }
        else if (inputChannel.GetInputChannelId() == AzFramework::InputDeviceMouse::SystemCursorPosition)
        {
            //hasBeenConsumed = true;
            const auto* pos2D = inputChannel.GetCustomData<AzFramework::InputChannel::PositionData2D>();

            if (!pos2D) //May be null in some circumstances.
            {
                return;
            }
            
            const float normalizedX = AZ::GetClamp(pos2D->m_normalizedPosition.GetX(), 0.0f, 1.0f);
            const float screenPosX = normalizedX * static_cast<float>(m_viewportSize.m_width);

            const float normalizedY = AZ::GetClamp(pos2D->m_normalizedPosition.GetY(), 0.0f, 1.0f);
            const float screenPosY = normalizedY * static_cast<float>(m_viewportSize.m_height);

            m_currentMousePos.Set(screenPosX, screenPosY);
            
            //m_currentMousePos.SetX(inputChannel.GetValue());
            //AZ_Printf(LogName, "MousePos=%0.3f,%0.3f.\n", m_currentMousePos.GetX(), m_currentMousePos.GetY());

        }
        //else if (inputChannel.GetInputChannelId() == AzFramework::InputDeviceMouse::Movement::X)
        //{
        //    hasBeenConsumed = true;
        //    const auto& mouseDevice = static_cast<const AzFramework::InputDeviceMouse&>(inputChannel.GetInputDevice());
        //    m_currentMousePos = mouseDevice.GetSystemCursorPositionNormalized();
        //
        //    //m_currentMousePos.SetX(inputChannel.GetValue());
        //    AZ_Printf(LogName, "MousePos=%0.1f,%0.1f.\n", m_currentMousePos.GetX(), m_currentMousePos.GetY());
        //}
        //else if (inputChannel.GetInputChannelId() == AzFramework::InputDeviceMouse::Movement::Y)
        //{
        //    hasBeenConsumed = true;
        //    const auto& mouseDevice = static_cast<const AzFramework::InputDeviceMouse&>(inputChannel.GetInputDevice());
        //    m_currentMousePos = mouseDevice.GetSystemCursorPositionNormalized();
        //    //m_currentMousePos.SetY(inputChannel.GetValue());
        //    AZ_Printf(LogName, "MousePos=%0.1f,%0.1f.\n", m_currentMousePos.GetX(), m_currentMousePos.GetY());
        //}
    }

    void RenderJoySystemComponent::OnKeyboardChannelEvent([[maybe_unused]] const AzFramework::InputChannel& inputChannel, [[maybe_unused]] bool& hasBeenConsumed)
    {
        if (m_keyboardTextureMgr)
        {
            m_keyboardTextureMgr->OnKeyboardChannelEvent(inputChannel, hasBeenConsumed);
        }
    }

    ////////////////////////////////////////////////////////////////////////
    // IKeyboardComponentsManager interface implementation START
    void RenderJoySystemComponent::RegisterKeyboardComponent(AZ::EntityId entityId)
    {
        if (AZStd::find(m_registeredKeyboardComponents.cbegin(), m_registeredKeyboardComponents.cend(), entityId) == m_registeredKeyboardComponents.cend())
        {
            m_registeredKeyboardComponents.push_back(entityId);
        }
        if (!m_keyboardTextureMgr && !m_registeredKeyboardComponents.empty())
        {
            m_keyboardTextureMgr = AZStd::make_unique<RenderJoyKeyboardTextureManager>();
        }
    }

    void RenderJoySystemComponent::UnregisterKeyboardComponent(AZ::EntityId entityId)
    {
        AZStd::erase_if(m_registeredKeyboardComponents
            , [&](const AZ::EntityId& eId) {
                return eId == entityId;
            });
        if (m_keyboardTextureMgr && m_registeredKeyboardComponents.empty())
        {
            m_keyboardTextureMgr.reset();
        }
    }

    void RenderJoySystemComponent::UpdateClearKeyPressedMilliseconds(uint32_t milliSeconds)
    {
        if (m_keyboardTextureMgr)
        {
            m_keyboardTextureMgr->m_maxWaitTimeToClearKeyPressedMilliseconds = milliSeconds;
        }
    }

    AZ::Data::Instance<AZ::RPI::Image> RenderJoySystemComponent::GetKeyboardTexture()
    {
        return m_keyboardTextureMgr ? m_keyboardTextureMgr->m_texture : nullptr;
    }
    // IKeyboardComponentsManager interface implementation END
    ////////////////////////////////////////////////////////////////////////

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
