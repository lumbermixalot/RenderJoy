/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/IO/FileIO.h>
#include <GraphCanvas/Widgets/NodePalette/TreeItems/NodePaletteTreeItem.h>

#include <QMessageBox>

#include "RenderJoyCanvasWindow.h"
//#include "MaterialCanvasViewportContent.h>

namespace RenderJoy
{
    RenderJoyCanvasWindow::RenderJoyCanvasWindow(QWidget* parent)
        : Base(ToolId, "RenderJoyCanvasWindow", parent)
        , m_styleManager(ToolId, "RenderJoy/StyleSheet/renderjoy_style.json")
    {
        //m_assetBrowser->SetFilterState("", AZ::RPI::StreamingImageAsset::Group, true);
        //m_assetBrowser->SetFilterState("", AZ::RPI::MaterialAsset::Group, true);

        m_toolBar = new AtomToolsFramework::EntityPreviewViewportToolBar(m_toolId, this);
        addToolBar(m_toolBar);

        m_documentInspector = new AtomToolsFramework::AtomToolsDocumentInspector(m_toolId, this);
        m_documentInspector->SetDocumentSettingsPrefix("/RenderJoy/PassEditor/DocumentInspector");
        AddDockWidget("Inspector", m_documentInspector, Qt::RightDockWidgetArea);

        AddDockWidget("MiniMap", aznew GraphCanvas::MiniMapDockWidget(m_toolId, this), Qt::RightDockWidgetArea);

        m_bookmarkDockWidget = aznew GraphCanvas::BookmarkDockWidget(m_toolId, this);
        AddDockWidget("Bookmarks", m_bookmarkDockWidget, Qt::BottomDockWidgetArea);

        GraphCanvas::NodePaletteConfig nodePaletteConfig;
        nodePaletteConfig.m_rootTreeItem = m_graphViewConfig.m_createNodeTreeItemsFn(m_toolId);
        nodePaletteConfig.m_editorId = m_toolId;
        nodePaletteConfig.m_mimeType = m_graphViewConfig.m_nodeMimeType.c_str();
        nodePaletteConfig.m_isInContextMenu = false;
        nodePaletteConfig.m_saveIdentifier = m_graphViewConfig.m_nodeSaveIdentifier;

        m_nodePalette = aznew GraphCanvas::NodePaletteDockWidget(this, "Node Palette", nodePaletteConfig);
        AddDockWidget("Node Palette", m_nodePalette, Qt::LeftDockWidgetArea);

        AZStd::array<char, AZ::IO::MaxPathLength> unresolvedPath;
        AZ::IO::FileIOBase::GetInstance()->ResolvePath(m_graphViewConfig.m_translationPath.c_str(), unresolvedPath.data(), unresolvedPath.size());

        QString translationFilePath(unresolvedPath.data());
        if (m_translator.load(QLocale::Language::English, translationFilePath))
        {
            if (!qApp->installTranslator(&m_translator))
            {
                AZ_Warning(LogName, false, "Error installing translation %s!", unresolvedPath.data());
            }
        }
        else
        {
            AZ_Warning(LogName, false, "Error loading translation file %s", unresolvedPath.data());
        }

        OnDocumentOpened(AZ::Uuid::CreateNull());
    }

    RenderJoyCanvasWindow::~RenderJoyCanvasWindow()
    {
    }

    void RenderJoyCanvasWindow::OnDocumentOpened(const AZ::Uuid& documentId)
    {
        Base::OnDocumentOpened(documentId);
        m_documentInspector->SetDocumentId(documentId);
    }

    void RenderJoyCanvasWindow::OnDocumentCleared(const AZ::Uuid& documentId)
    {
        Base::OnDocumentCleared(documentId);
        m_documentInspector->SetDocumentId(documentId);
    }

    void RenderJoyCanvasWindow::OnDocumentError(const AZ::Uuid& documentId)
    {
        Base::OnDocumentError(documentId);
        m_documentInspector->SetDocumentId(documentId);
    }

    void RenderJoyCanvasWindow::ResizeViewportRenderTarget(uint32_t width, uint32_t height)
    {
        AZ_Warning(LogName, false, "Error %s for size (%u,%u) is unsupported", __FUNCTION__, width, height);
        //QSize requestedViewportSize = QSize(width, height) / devicePixelRatioF();
        //QSize currentViewportSize = m_materialViewport->size();
        //QSize offset = requestedViewportSize - currentViewportSize;
        //QSize requestedWindowSize = size() + offset;
        //resize(requestedWindowSize);
        //
        //AZ_Assert(
        //    m_materialViewport->size() == requestedViewportSize,
        //    "Resizing the window did not give the expected viewport size. Requested %d x %d but got %d x %d.",
        //    requestedViewportSize.width(), requestedViewportSize.height(), m_materialViewport->size().width(),
        //    m_materialViewport->size().height());
        //
        //[[maybe_unused]] QSize newDeviceSize = m_materialViewport->size();
        //AZ_Warning(
        //    "Material Canvas",
        //    static_cast<uint32_t>(newDeviceSize.width()) == width && static_cast<uint32_t>(newDeviceSize.height()) == height,
        //    "Resizing the window did not give the expected frame size. Requested %d x %d but got %d x %d.", width, height,
        //    newDeviceSize.width(), newDeviceSize.height());
    }

    void RenderJoyCanvasWindow::LockViewportRenderTargetSize(uint32_t width, uint32_t height)
    {
        //m_materialViewport->LockRenderTargetSize(width, height);
        AZ_Warning(LogName, false, "Error %s for size (%u,%u) is unsupported", __FUNCTION__, width, height);
    }

    void RenderJoyCanvasWindow::UnlockViewportRenderTargetSize()
    {
        //m_materialViewport->UnlockRenderTargetSize();
        AZ_Warning(LogName, false, "Error %s is unsupported", __FUNCTION__);
    }

    AZStd::string RenderJoyCanvasWindow::GetHelpDialogText() const
    {
        return R"(<html><head/><body>
            <p><h3><u>Welcome to RenderJoy</u></h3></p>
            <p><b>Have Fun...</b> in moderation.</p>
            </body></html>)";
    }
} // namespace RenderJoy

#include "moc_RenderJoyCanvasWindow.cpp"
