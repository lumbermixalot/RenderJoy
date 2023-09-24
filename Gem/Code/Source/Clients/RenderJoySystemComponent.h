/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>

#include <AzFramework/Input/Buses/Notifications/InputChannelNotificationBus.h>
#include <AzFramework/Windowing/WindowBus.h>

#include <Atom/RPI.Reflect/Asset/AssetUtils.h>
#include <Atom/RPI.Public/Image/StreamingImage.h>

#include <RenderJoy/RenderJoyBus.h>
#include <RenderJoy/RenderJoyFeatureProcessorInterface.h>
#include <RenderJoy/IRenderJoySrgDataProvider.h>
#include <RenderJoy/IKeyboardComponentsManager.h>

#include "RenderJoyTemplatesFactory.h"
#include "RenderJoyKeyboardTextureManager.h"

namespace AZ
{
    namespace RPI
    {
        class Scene;
    }
}

namespace RenderJoy
{
    class RenderJoySystemComponent
        : public AZ::Component
        , public AZ::SystemTickBus::Handler // For feature processor creation/destruction.
        , public AZ::TickBus::Handler // For keeping track of time during rendering.
        , public RenderJoyRequestBus::Handler
        , public IRenderJoySrgDataProvider
        , public IKeyboardComponentsManager
        , public RenderJoyNotificationBus::Handler
        , public AzFramework::InputChannelNotificationBus::Handler // To listen to mouse and keyboard.
    {
    public:
        AZ_COMPONENT_DECL(RenderJoySystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        RenderJoySystemComponent();
        ~RenderJoySystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // RenderJoyRequestBus interface implementation START
        bool AddRenderJoyParentPass(AZ::EntityId parentPassEntityId, AZ::EntityId passBusEntity) override;
        bool RemoveRenderJoyParentPass(AZ::EntityId parentPassEntityId) override;
        AZStd::vector<AZ::EntityId> GetParentPassEntities() const override;
        AZStd::shared_ptr<AZ::RPI::PassRequest> GetPassRequest(AZ::EntityId parentPassEntityId) const override;
        AZ::Name GetBillboardPassName(AZ::EntityId parentPassEntityId) const override;
        AZ::Name GetShaderPassName(AZ::EntityId shaderPassEntityId) const override;
        AZ::Data::Instance<AZ::RPI::Image> GetInvalidParentPassTexture() const override;
        AZ::Data::Instance<AZ::RPI::Image> GetDefaultInputTexture(uint32_t channelIndex) const override;
        AZ::EntityId GetEntityIdFromPassName(const AZ::Name& passName) const override;
        void RecreateAllPasses(AZ::EntityId shaderPassTriggerEntityId) override;
        // RenderJoyRequestBus interface implementation END
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // IRenderJoySrgDataProvider interface implementation START
        float GetTime() override;
        float GetTimeDelta() override;
        int GetFramesCount() override;
        float GetFramesPerSecond() override;
        void GetMouseData(AZ::Vector2& currentPos, AZ::Vector2& clickPos, bool& isLeftButtonDown, bool& isLeftButtonClick) override;
        void ResetFrameCounter(int newValue) override;
        // IRenderJoySrgDataProvider interface implementation END
        ////////////////////////////////////////////////////////////////////////


        ////////////////////////////////////////////////////////////////////////
        // IKeyboardComponentsManager interface implementation START
        void RegisterKeyboardComponent(AZ::EntityId entityId) override;
        void UnregisterKeyboardComponent(AZ::EntityId entityId) override;
        void UpdateClearKeyPressedMilliseconds(uint32_t milliSeconds) override;
        AZ::Data::Instance<AZ::RPI::Image> GetKeyboardTexture() override;
        // IKeyboardComponentsManager interface implementation END
        ////////////////////////////////////////////////////////////////////////


        //////////////////////////////////////////////////////////////////////////
        // SystemTickBus
        void OnSystemTick() override;
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // TickBus
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        //////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////
        // RenderJoyNotificationBus::Handler overrides START
        void OnFeatureProcessorActivated() override;
        void OnFeatureProcessorDeactivated() override;
        // RenderJoyNotificationBus::Handler overrides END
        ///////////////////////////////////////////////////////////


        ///////////////////////////////////////////////////////////
        // InputChannelNotificationBus::Handler overrides START
        void OnInputChannelEvent(const AzFramework::InputChannel& inputChannel, bool& hasBeenConsumed) override;
        // InputChannelNotificationBus::Handler overrides END
        ///////////////////////////////////////////////////////////

        bool m_shouldRecreateFeatureProcessor = true;

    private:
        static constexpr char LogName[] = "RenderJoySystemComponent";
        static constexpr char InvalidPipelineTexturePath[] =  "textures/renderjoy/invalidpipeline.png.streamingimage"; //"Textures/RenderJoy/InvalidPipeline.png"; 

        void DestroyFeatureProcessor();
        void CreateFeatureProcessor();

        void OnMouseChannelEvent(const AzFramework::InputChannel& inputChannel, bool& hasBeenConsumed);
        void OnKeyboardChannelEvent(const AzFramework::InputChannel& inputChannel, bool& hasBeenConsumed);

        // It is assumed that all RenderJoy related passes go to the same scene.
        AZ::RPI::Scene* m_scenePtr = nullptr;
        RenderJoyFeatureProcessorInterface* m_featureProcessor = nullptr;

        mutable AZ::RPI::AssetUtils::AsyncAssetLoader m_asyncAssetLoader;
        mutable bool m_asyncLoadStarted = false;
        mutable AZ::Data::Instance<AZ::RPI::StreamingImage> m_invalidParentPassTexture;

        RenderJoyTemplatesFactory m_templatesFactory;

        // We need to queue removal and addition because we have to serially and safely destroy and recreate the feature processor.
        // Key: parentPassEntityId, Value: passBusEntity
        // If passBusEntity is invalid, then we will remove parentPassEntityId.
        AZStd::map<AZ::EntityId, AZ::EntityId> m_entitiesToProcess;

        int32_t m_frameCounter = 0;
        float m_runTime = 0.0f;
        float m_frameTimeDelta = 0.016f;
        float m_oneSecondMark = 0.0; // Accumulates time until second and calculates fps.
        int32_t m_frameCounterStamp = 0;
        float m_fps = 60.0f;

        // Mouse state management, per ShaderToy's convention
        AZ::Vector2 m_currentMousePos = AZ::Vector2(0.0f, 0.0f);
        AZ::Vector2 m_clickMousePos = AZ::Vector2(0.0f, 0.0f);
        bool m_isLeftButtonDown = false;
        bool m_isLeftButtonClick = false;
        float m_timeSinceLastClick = 0;
        // Need to keep track of the viewport size, as the mouse position are in pixel locations
        // and the input APIs report normalized values.
        // Define width and height from the viewport size.
        AzFramework::WindowSize m_viewportSize;

        AZStd::unique_ptr<RenderJoyKeyboardTextureManager> m_keyboardTextureMgr;
        AZStd::vector<AZ::EntityId> m_registeredKeyboardComponents;
    };

} // namespace RenderJoy
