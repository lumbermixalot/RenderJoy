/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#if !defined(Q_MOC_RUN)
#include <AzCore/Asset/AssetCommon.h>
#include <AzFramework/Windowing/WindowBus.h>

AZ_PUSH_DISABLE_WARNING(4251 4800, "-Wunknown-warning-option") // disable warnings spawned by QT
#include <QWidget>
AZ_POP_DISABLE_WARNING

#include <AtomToolsFramework/Viewport/RenderViewportWidget.h>
#endif

namespace Ui
{
    class RenderJoyViewportWidget;
}

namespace AZ
{
    namespace RPI
    {
        class WindowContext;
    }
}

namespace RenderJoy
{
    class RenderJoyViewportRenderer;

    class RenderJoyViewportWidget
        : public AtomToolsFramework::RenderViewportWidget
    {
    public:
        RenderJoyViewportWidget(QWidget* parent = nullptr);

        QScopedPointer<Ui::RenderJoyViewportWidget> m_ui;
        AZStd::shared_ptr<RenderJoyViewportRenderer> m_renderer;

    protected:
        bool eventFilter(QObject*, QEvent*) override;

    };
} // namespace RenderJoy
