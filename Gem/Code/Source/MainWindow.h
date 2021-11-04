/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#if !defined(Q_MOC_RUN)

// AZ
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/PlatformDef.h>
#include <AzQtComponents/Components/StyledDockWidget.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzCore/Component/TickBus.h>

// Qt
#include <QtWidgets/QMainWindow>

#endif

namespace RenderJoy
{
    class RenderJoyViewportWidget;

    class MainWindow
        : public QMainWindow
        , public AZ::TickBus::Handler
    {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow() override;

        // AZ::TickBus::Handler interface overrides...
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        QSize sizeHint() const override
        {
            return QSize(800, 450);
        }


    private:

        RenderJoyViewportWidget* m_viewport = nullptr;
    };
} 
