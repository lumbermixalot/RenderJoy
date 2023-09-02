/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#undef RC_INVOKED

#include <AzCore/Component/Entity.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Console/IConsole.h>

#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>

#include <AtomCore/Instance/InstanceDatabase.h>

#include <Atom/RHI/RHISystemInterface.h>

#include <Atom/RPI.Public/WindowContext.h>
#include <Atom/RPI.Public/Pass/Specific/SwapChainPass.h>
#include <Atom/RPI.Public/RPISystemInterface.h>
#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Image/StreamingImage.h>
#include <Atom/RPI.Reflect/Asset/AssetUtils.h>
#include <Atom/RPI.Public/View.h>
#include <Atom/RPI.Public/Culling.h>
#include <Atom/RPI.Public/Pass/PassSystem.h>

#include "RenderJoyViewportRenderer.h"

namespace RenderJoy
{
    AZ_CVAR(
        int, rj_frame_counter, 0,
        [](const int& value) {
            auto ptr = AZ::Interface<IRenderJoySrgDataProvider>::Get();
            if (!ptr)
            {
                return;
            }
            ptr->ResetFrameCounter(value);
        },
        AZ::ConsoleFunctorFlags::Null,
        "Resets the frame counter to the given value.");

    RenderJoyViewportRenderer::RenderJoyViewportRenderer(AZStd::shared_ptr<AZ::RPI::WindowContext> windowContext)
        : m_windowContext(windowContext)
    {
        using namespace AZ;
        using namespace Data;

        m_view = AZ::RPI::View::CreateView(AZ::Name("RenderJoyViewTag"), AZ::RPI::View::UsageNone);

        auto passSystem = AZ::RPI::PassSystemInterface::Get();
        if (!m_mainPassCreator.Create(passSystem))
        {
            m_mainPassCreator.RemoveTemplates(passSystem);
            return;
        }

        // Create and register a scene
        AZ::RPI::SceneDescriptor sceneDesc;
        m_scene = AZ::RPI::Scene::CreateScene(sceneDesc);

        // Create a render pipeline from the specified asset for the window context and add the pipeline to the scene
        AZ::Data::Asset<AZ::RPI::AnyAsset> pipelineAsset = AZ::RPI::AssetUtils::LoadAssetByProductPath<AZ::RPI::AnyAsset>(m_defaultPipelineAssetPath.c_str(), AZ::RPI::AssetUtils::TraceLevel::Error);
        m_renderPipeline = AZ::RPI::RenderPipeline::CreateRenderPipelineForWindow(pipelineAsset, *m_windowContext.get());
        m_renderPipeline->UpdatePasses();
        m_renderPipeline->SetDefaultView(m_view);
        pipelineAsset.Release();
        m_scene->AddRenderPipeline(m_renderPipeline);

        // Currently the scene has to be activated after render pipeline was added so some feature processors (i.e. imgui) can be initialized properly 
        // with pipeline's pass information. 
        m_scene->Activate();

        AZ::RPI::RPISystemInterface::Get()->RegisterScene(m_scene);
    }

    RenderJoyViewportRenderer::~RenderJoyViewportRenderer()
    {
        AZ::Data::AssetBus::Handler::BusDisconnect();

        // Need to call this to delete all the RenderJoy Passes from memory
        // so We don't get errors later when calling m_mainPassCreator.RemoveTemplates()
        if (m_renderPipeline)
        {
            m_renderPipeline->RemoveFromScene();
            m_renderPipeline = nullptr;
        }

        if (m_scene)
        {
            AZ::RPI::RPISystemInterface::Get()->UnregisterScene(m_scene);
            m_scene = nullptr;
        }
 
        //Passes have been removed. Let's remove the templates we created.
        m_mainPassCreator.RemoveTemplates(AZ::RPI::PassSystemInterface::Get());
    }

    void RenderJoyViewportRenderer::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {

    }

    ///////////////////////////////////////////////////////////////
    // AzFramework::ViewportControllerInterface Overrides
    void RenderJoyViewportRenderer::UpdateViewport(const AzFramework::ViewportControllerUpdateEvent& event)
    {
        m_simulateTimeDelta = event.m_deltaTime.count();
        m_simulateTime += m_simulateTimeDelta;
        m_framesCount++;


        m_oneSecondCounter += m_simulateTimeDelta;
        m_frameCountInThisSecond += 1.0f;
        if (m_oneSecondCounter >= 1.0f)
        {
            m_framesPerSeconds = m_frameCountInThisSecond;
            m_oneSecondCounter = 0.0f;
            m_frameCountInThisSecond = 0.0f;
            //AZ_TracePrintf(LogName, "iFrame=%d, iFrameRate=%d\n", m_framesCount, aznumeric_cast<int>(m_framesPerSeconds));
        }


        if ( m_isLeftButtonClick && (m_simulateTime > (m_timeSinceLastClick + 1.0)) )
        {
            m_isLeftButtonClick = false;
        }


    }
    ///////////////////////////////////////////////////////////////

    void RenderJoyViewportRenderer::OnMouseLeftButtonDown(bool state, int posX, int posY)
    {
        if (state)
        {
            m_isLeftButtonDown = true;
            m_isLeftButtonClick = true;
            m_timeSinceLastClick = m_simulateTime;
            m_currentMousePos.Set(aznumeric_cast<float>(posX), aznumeric_cast<float>(posY));
            m_clickMousePos = m_currentMousePos;
        }
        else
        {
            m_isLeftButtonDown = false;
            m_isLeftButtonClick = false;
            m_currentMousePos.Set(aznumeric_cast<float>(posX), aznumeric_cast<float>(posY));
        }
    }

    void RenderJoyViewportRenderer::OnMouseMove(int posX, int posY)
    {
        if (!m_isLeftButtonDown)
        {
            return;
        }
        m_currentMousePos.Set(aznumeric_cast<float>(posX), aznumeric_cast<float>(posY));
    }

}
