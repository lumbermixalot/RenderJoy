/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#if !defined(Q_MOC_RUN)
#include <AtomToolsFramework/Document/AtomToolsDocumentInspector.h>
#include <AtomToolsFramework/Document/AtomToolsDocumentMainWindow.h>
#include <AtomToolsFramework/EntityPreviewViewport/EntityPreviewViewportToolBar.h>
#include <AtomToolsFramework/GraphView/GraphViewConfig.h>
#include <GraphCanvas/Styling/StyleManager.h>
#include <GraphCanvas/Widgets/Bookmarks/BookmarkDockWidget.h>
#include <GraphCanvas/Widgets/MiniMapGraphicsView/MiniMapGraphicsView.h>
#include <GraphCanvas/Widgets/NodePalette/NodePaletteDockWidget.h>
#include <GraphCanvas/Widgets/NodePalette/NodePaletteWidget.h>
#include <RenderJoy/RenderJoyCanvasWindowRequestBus.h>

#include <QTranslator>
#endif

namespace RenderJoy
{
    //! RenderJoyCanvasWindow
    class RenderJoyCanvasWindow : public AtomToolsFramework::AtomToolsDocumentMainWindow,
        public RenderJoyCanvasWindowRequestBus::Handler
    {
        Q_OBJECT
    public:
        AZ_CLASS_ALLOCATOR(RenderJoyCanvasWindow, AZ::SystemAllocator, 0);

        using Base = AtomToolsFramework::AtomToolsDocumentMainWindow;

        RenderJoyCanvasWindow(QWidget* parent = 0);
        ~RenderJoyCanvasWindow();

        static constexpr char LogName[] = "RenderJoyCanvasWindow";
        static constexpr AZ::Crc32 ToolId = AZ_CRC_CE("RenderJoy");

    protected:
        // AtomToolsFramework::AtomToolsMainWindowRequestBus::Handler overrides...
        void ResizeViewportRenderTarget(uint32_t width, uint32_t height) override;
        void LockViewportRenderTargetSize(uint32_t width, uint32_t height) override;
        void UnlockViewportRenderTargetSize() override;

        // AtomToolsFramework::AtomToolsDocumentNotificationBus::Handler overrides...
        void OnDocumentOpened(const AZ::Uuid& documentId) override;
        void OnDocumentCleared(const AZ::Uuid& documentId) override;
        void OnDocumentError(const AZ::Uuid& documentId) override;

        // AtomToolsFramework::AtomToolsDocumentMainWindow overrides...
        AZStd::string GetHelpDialogText() const override;

        // RenderJoyCanvasWindowRequestBus overrides...
        RenderJoyCanvasWindow* GetRenderJoyCanvasWindow() override { return this; }

    private:
        AtomToolsFramework::AtomToolsDocumentInspector* m_documentInspector = {};
        AtomToolsFramework::EntityPreviewViewportToolBar* m_toolBar = {};
        AtomToolsFramework::GraphViewConfig m_graphViewConfig;
        GraphCanvas::BookmarkDockWidget* m_bookmarkDockWidget = {};
        GraphCanvas::NodePaletteDockWidget* m_nodePalette = {};
        GraphCanvas::StyleManager m_styleManager;
        QTranslator m_translator;
    };
} // namespace RenderJoy
