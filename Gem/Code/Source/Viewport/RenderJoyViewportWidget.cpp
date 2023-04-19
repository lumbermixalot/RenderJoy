/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Atom/RHI/Device.h>
#include <Atom/RPI.Public/RPISystemInterface.h>
#include <Atom/RHI/RHISystemInterface.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/WindowContext.h>

AZ_PUSH_DISABLE_WARNING(4251 4800, "-Wunknown-warning-option") // disable warnings spawned by QT
#include <QWindow>
#include <QMouseEvent>
#include <Source/Viewport/ui_RenderJoyViewportWidget.h>
AZ_POP_DISABLE_WARNING

#include <AzCore/PlatformIncl.h>
#include <AzFramework/Viewport/ViewportControllerList.h>


#include <RenderJoy/RenderJoyKeyboardBus.h>
#include "RenderJoyViewportRenderer.h"
#include "RenderJoyViewportWidget.h"

namespace RenderJoy
{

    RenderJoyViewportWidget::RenderJoyViewportWidget(QWidget* parent)
        : AtomToolsFramework::RenderViewportWidget(parent)
        , m_ui(new Ui::RenderJoyViewportWidget)
    {
        m_ui->setupUi(this);

        m_renderer = AZStd::make_shared<RenderJoyViewportRenderer>(GetViewportContext()->GetWindowContext());
        GetControllerList()->Add(m_renderer);

        installEventFilter(this);
    }

    bool RenderJoyViewportWidget::eventFilter(QObject* obj, QEvent* ev)
    {
        const QEvent::Type evType = ev->type();
        if (evType == QEvent::MouseButtonPress)
        {
            QMouseEvent* mouseEvent = (QMouseEvent*)ev;
            Qt::MouseButton button = mouseEvent->button();
            if (button == Qt::LeftButton)
            {
                m_renderer->OnMouseLeftButtonDown(true, mouseEvent->x(), mouseEvent->y());
            }
        }
        else if (evType == QEvent::MouseMove)
        {
            QMouseEvent* mouseEvent = (QMouseEvent*)ev;
            m_renderer->OnMouseMove(mouseEvent->x(), mouseEvent->y());
        }
        else if (evType == QEvent::MouseButtonRelease)
        {
            QMouseEvent* mouseEvent = (QMouseEvent*)ev;
            Qt::MouseButton button = mouseEvent->button();
            if (button == Qt::LeftButton)
            {
                m_renderer->OnMouseLeftButtonDown(false, mouseEvent->x(), mouseEvent->y());
            }
        }
        else if (evType == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = (QKeyEvent*)ev;
            if (!keyEvent->isAutoRepeat())
            {
                RenderJoyKeyboardNotifications::KeyCodeInfo keyCodeInfo = {
                    keyEvent->nativeVirtualKey(), keyEvent->nativeScanCode(), keyEvent->key()};
                RenderJoyKeyboardNotificationBus::Broadcast(&RenderJoyKeyboardNotifications::OnKeyDown, keyCodeInfo);

                // char key = 0xff & keyEvent->key();
                // char nativeScanCode = 0xff & keyEvent->nativeScanCode();
                // char virtualScancode = 0xff & keyEvent->nativeVirtualKey();
                // AZ_TracePrintf(
                //    "RenderJoyViewportWidget", "Pressed with key=[%c, 0x%02X], nativeScanCode=[%c, 0x%02X], virtualScancode=[%c, 0x%02X]",
                //    key, key, nativeScanCode, nativeScanCode, virtualScancode, virtualScancode);
            }
        }
        else if (evType == QEvent::KeyRelease)
        {
            QKeyEvent* keyEvent = (QKeyEvent*)ev;
            if (!keyEvent->isAutoRepeat())
            {
                RenderJoyKeyboardNotifications::KeyCodeInfo keyCodeInfo = {
                    keyEvent->nativeVirtualKey(), keyEvent->nativeScanCode(), keyEvent->key()};
                RenderJoyKeyboardNotificationBus::Broadcast(&RenderJoyKeyboardNotifications::OnKeyUp, keyCodeInfo);
                // char key = 0xff & keyEvent->key();
                // char nativeScanCode = 0xff & keyEvent->nativeScanCode();
                // char virtualScancode = 0xff & keyEvent->nativeVirtualKey();
                // AZ_TracePrintf(
                //    "RenderJoyViewportWidget", "Pressed with key=[%c, 0x%02X], nativeScanCode=[%c, 0x%02X], virtualScancode=[%c, 0x%02X]",
                //    key, key, nativeScanCode, nativeScanCode, virtualScancode, virtualScancode);
            }
        }

        return QWidget::eventFilter(obj, ev);
    }

} // namespace RenderJoy

#include <Source/Viewport/moc_RenderJoyViewportWidget.cpp>
