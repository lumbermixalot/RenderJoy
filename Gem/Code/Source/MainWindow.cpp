/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "MainWindow.h"

// AZ
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/Utils.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzQtComponents/Buses/ShortcutDispatch.h>
#include <AzToolsFramework/UI/ComponentPalette/ComponentPaletteUtil.hxx>
#include <AzToolsFramework/Undo/UndoSystem.h>

#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/WindowContext.h>

// Qt
#include <QApplication>
#include <QStringList>
#include <QTimer>
#include <QVBoxLayout>

//Atom
#include <Atom/RHI/Factory.h>

//RenderJoy
#include "Viewport/RenderJoyViewportWidget.h"
#include "Viewport/RenderJoyViewportRenderer.h"
#include <RenderJoy/RenderJoySettingsBus.h>

namespace RenderJoy
{
    MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent
            , Qt::Window
            | Qt::WindowSystemMenuHint
            | Qt::WindowCloseButtonHint
            | Qt::WindowFullscreenButtonHint
            | Qt::BypassGraphicsProxyWidget
            | Qt::WindowStaysOnTopHint)
    {
        AZ::Name apiName = AZ::RHI::Factory::Get().GetName();
        if (!apiName.IsEmpty())
        {
            QString title = QString{ "RenderJoy (%1)" }.arg(apiName.GetCStr());
            setWindowTitle(title);
        }
        else
        {
            AZ_Assert(false, "Render API name not found");
            setWindowTitle("RenderJoy");
        }

        m_viewport = new RenderJoyViewportWidget(this);
        m_viewport->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        AZ::Vector2 size(1280, 720);
        RenderJoySettingsRequestBus::BroadcastResult(size, &RenderJoySettingsRequests::GetRenderTargetSize);
        uint32_t width = aznumeric_cast<uint32_t>(AZStd::ceilf(size.GetX()));
        uint32_t height = aznumeric_cast<uint32_t>(AZStd::ceilf(size.GetY()));
        m_viewport->LockRenderTargetSize(width, height);
        setCentralWidget(m_viewport);
        AZ::TickBus::Handler::BusConnect();
    }

    MainWindow::~MainWindow()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void MainWindow::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        auto viewportContext = m_viewport->GetViewportContext();
        viewportContext->SetRenderScene(m_viewport->m_renderer->GetScene());
        viewportContext->RenderTick();
    }

}// namespace RenderJoy

#include <Source/moc_MainWindow.cpp>
