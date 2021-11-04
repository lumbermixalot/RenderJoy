/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Interface/Interface.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Component/TransformBus.h>

#include <AzFramework/Viewport/ViewportControllerInterface.h>

#include <AtomCore/Instance/Instance.h>

#include <Atom/RPI.Public/Base.h>

#include <RenderJoy/IRenderJoySrgDataProvider.h>
#include "RenderJoyTrianglePass.h"
#include "RenderJoyMainPassTemplateCreator.h"

//#include <RenderJoy/Viewport/RenderJoyViewportNotificationBus.h>
//#include "InputController/RenderJoyViewportInputController.h"

namespace AZ
{
    namespace RPI
    {
        class WindowContext;
    }
}

namespace RenderJoy
{
    //! Provides backend logic for MaterialViewport
    //! Sets up a scene, camera, loads the model, and applies texture
    class RenderJoyViewportRenderer
        : public AZ::Data::AssetBus::Handler
        , public AzFramework::ViewportControllerInterface
        , public AZ::Interface<IRenderJoySrgDataProvider>::Registrar
    {
    public:
        AZ_CLASS_ALLOCATOR(RenderJoyViewportRenderer, AZ::SystemAllocator, 0);

        RenderJoyViewportRenderer(AZStd::shared_ptr<AZ::RPI::WindowContext> windowContext);
        ~RenderJoyViewportRenderer();

        AZ::RPI::ScenePtr GetScene() { return m_scene; }

        void OnMouseLeftButtonDown(bool state, int posX, int posY);
        void OnMouseMove(int posX, int posY);

    private:

        // AZ::Data::AssetBus::Handler interface overrides...
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;

        ///////////////////////////////////////////////////////////////
        // IRenderJoySrgDataProvider Overrides
        float GetTime() override { return  m_simulateTime; }
        float GetTimeDelta() override { return  m_simulateTimeDelta; }
        int GetFramesCount() override { return m_framesCount; }
        float GetFramesPerSecond() override { return m_framesPerSeconds; }
        void GetMouseData(AZ::Vector2& currentPos, AZ::Vector2& clickPos, bool& isLeftButtonDown, bool& isLeftButtonClick) override {
            currentPos = m_currentMousePos;
            clickPos = m_clickMousePos;
            isLeftButtonDown = m_isLeftButtonDown;
            isLeftButtonClick = m_isLeftButtonClick;
        }
        void ResetFrameCounter(int newValue)
        {
            AZ_TracePrintf(LogName, "Frame counter will be reset from %d to %d", m_framesCount, newValue);
            m_framesCount = newValue;
        }
        ///////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////
        // AzFramework::ViewportControllerInterface Overrides
        bool HandleInputChannelEvent([[maybe_unused]] const AzFramework::ViewportControllerInputEvent& event) override { return false; }
        void UpdateViewport([[maybe_unused]] const AzFramework::ViewportControllerUpdateEvent& event) override;
        void RegisterViewportContext([[maybe_unused]] AzFramework::ViewportId viewport) override {}
        void UnregisterViewportContext([[maybe_unused]] AzFramework::ViewportId viewport) override {}
        ///////////////////////////////////////////////////////////////

        AZStd::string m_defaultPipelineAssetPath = "passes/RenderJoyMainPipeline.azasset";
        RenderJoyMainPassTemplateCreator m_mainPassCreator;
        AZStd::shared_ptr<AZ::RPI::WindowContext> m_windowContext;
        AZ::RPI::RenderPipelinePtr m_renderPipeline;
        AZ::RPI::ScenePtr m_scene;

        // Dummy view, we don't need a view in RenderJoy, BUT the RPI::RenderPipeline needs one.
        AZ::RPI::ViewPtr m_view = nullptr;

        float m_simulateTime = 0;
        float m_simulateTimeDelta = 0;
        float m_timeSinceLastClick = 0;
        int m_framesCount = 0;
        float m_framesPerSeconds = 0.0f;

        // Variables to measure FPS
        float m_oneSecondCounter = 0.0; // Accumulates time until it reaches one second
        float m_frameCountInThisSecond = 0.0;

        AZ::Vector2 m_currentMousePos = AZ::Vector2(0.0f, 0.0f);
        AZ::Vector2 m_clickMousePos = AZ::Vector2(0.0f, 0.0f);
        bool m_isLeftButtonDown = false;
        bool m_isLeftButtonClick = false;

    };
} // namespace RenderJoy
