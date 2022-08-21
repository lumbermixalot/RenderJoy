/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/SerializeContext.h>

#include <AzToolsFramework/API/ViewPaneOptions.h>
#include <LyViewPaneNames.h>
#include <AtomToolsFramework/Document/AtomToolsDocumentSystemRequestBus.h>

#include "RenderJoyEditorSystemComponent.h"
#include "MainWindow.h"
#include "Canvas/RenderJoyCanvasWindow.h"
#include "Canvas/RenderJoyCanvasGraphView.h"
#include "Canvas/Document/RenderJoyCanvasDocument.h"
#include <RenderJoy/RenderJoySettingsBus.h>
#include <RenderJoy/RenderJoyCanvasWindowRequestBus.h>


namespace RenderJoy
{
    void RenderJoyEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyEditorSystemComponent, AZ::Component>()
                ->Version(0);
        }
    }

    RenderJoyEditorSystemComponent::RenderJoyEditorSystemComponent() = default;

    RenderJoyEditorSystemComponent::~RenderJoyEditorSystemComponent()
    {
        AzToolsFramework::CloseViewPane(RenderJoyName);
        AzToolsFramework::UnregisterViewPane(RenderJoyName);
    }

    void RenderJoyEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("RenderJoyEditorService"));
    }

    void RenderJoyEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("RenderJoyEditorService"));
    }

    void RenderJoyEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void RenderJoyEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void RenderJoyEditorSystemComponent::Activate()
    {
        InitDataForPassEditor();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
        RenderJoyCanvasSystemRequestBus::Handler::BusConnect();
    }

    void RenderJoyEditorSystemComponent::Deactivate()
    {
        RenderJoyCanvasSystemRequestBus::Handler::BusDisconnect();
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
    }

    void RenderJoyEditorSystemComponent::InitDataForPassEditor()
    {
        // Instantiate the dynamic node manager to register all dynamic node configurations and data types used in this tool
        m_dynamicNodeManager.reset(aznew AtomToolsFramework::DynamicNodeManager(RenderJoyCanvasWindow::ToolId));

        // Register all data types required by material canvas nodes with the dynamic node manager
        m_dynamicNodeManager->RegisterDataTypes({
            AZStd::make_shared<GraphModel::DataType>(AZ_CRC_CE("bool"), bool{}, "bool"),
            AZStd::make_shared<GraphModel::DataType>(AZ_CRC_CE("int"), int32_t{}, "int"),
            AZStd::make_shared<GraphModel::DataType>(AZ_CRC_CE("uint"), uint32_t{}, "uint"),
            AZStd::make_shared<GraphModel::DataType>(AZ_CRC_CE("float"), float{}, "float"),
            AZStd::make_shared<GraphModel::DataType>(AZ_CRC_CE("float2"), AZ::Vector2::CreateZero(), "float2"),
            AZStd::make_shared<GraphModel::DataType>(AZ_CRC_CE("float3"), AZ::Vector3::CreateZero(), "float3"),
            AZStd::make_shared<GraphModel::DataType>(AZ_CRC_CE("float4"), AZ::Vector4::CreateZero(), "float4"),
            AZStd::make_shared<GraphModel::DataType>(AZ_CRC_CE("color"), AZ::Color::CreateOne(), "color"),
            AZStd::make_shared<GraphModel::DataType>(AZ_CRC_CE("string"), AZStd::string{}, "string"),
        });

        // Search the project and gems for dynamic node configurations and register them with the manager
        m_dynamicNodeManager->LoadConfigFiles("renderjoynode");

        // Each graph document creates its own graph context but we want to use a shared graph context instead to avoid data duplication
        m_graphContext = AZStd::make_shared<GraphModel::GraphContext>(
            "RenderJoy Canvas", ".renderjoy", m_dynamicNodeManager->GetRegisteredDataTypes());
        m_graphContext->CreateModuleGraphManager();

        // This configuration data is passed through the main window and graph views to setup translation data, styling, and node palettes
        m_graphViewConfig.m_translationPath = "@products@/translation/renderjoy_en_us.qm";
        m_graphViewConfig.m_styleManagerPath = "RenderJoy/StyleSheet/renderjoy_style.json";
        m_graphViewConfig.m_nodeMimeType = "RenderJoy/node-palette-mime-event";
        m_graphViewConfig.m_nodeSaveIdentifier = "RenderJoy/ContextMenu";
        m_graphViewConfig.m_createNodeTreeItemsFn = [](const AZ::Crc32& toolId)
        {
            GraphCanvas::GraphCanvasTreeItem* rootTreeItem = {};
            AtomToolsFramework::DynamicNodeManagerRequestBus::EventResult(
                rootTreeItem, toolId, &AtomToolsFramework::DynamicNodeManagerRequestBus::Events::CreateNodePaletteTree);
            return rootTreeItem;
        };

        // Acquiring default material canvas document type info so that it can be customized before registration
        auto documentTypeInfo = RenderJoyCanvasDocument::BuildDocumentTypeInfo();

        // Overriding default document factory function to pass in a shared graph context
        documentTypeInfo.m_documentFactoryCallback = [this](const AZ::Crc32& toolId, const AtomToolsFramework::DocumentTypeInfo& documentTypeInfo)
        {
            return aznew RenderJoyCanvasDocument(toolId, documentTypeInfo, m_graphContext);
        };

        // Overriding documentview factory function to create graph view
        documentTypeInfo.m_documentViewFactoryCallback = [this](const AZ::Crc32& toolId, const AZ::Uuid& documentId)
        {
            RenderJoyCanvasWindow* window = nullptr;
            RenderJoyCanvasWindowRequestBus::BroadcastResult(window, &RenderJoyCanvasWindowRequests::GetRenderJoyCanvasWindow);
            return window->AddDocumentTab(documentId, aznew RenderJoyCanvasGraphView(toolId, documentId, m_graphViewConfig));
        };

        AtomToolsFramework::AtomToolsDocumentSystemRequestBus::Event(
            RenderJoyCanvasWindow::ToolId, &AtomToolsFramework::AtomToolsDocumentSystemRequestBus::Handler::RegisterDocumentType, documentTypeInfo);

        //m_viewportSettingsSystem.reset(aznew AtomToolsFramework::EntityPreviewViewportSettingsSystem(m_toolId));
    }

    ////////////////////////////////////////////////////////////////////////
    // AztoolsFramework::EditorEvents::Bus::Handler overrides
    void RenderJoyEditorSystemComponent::NotifyRegisterViews()
    {
        AZ::Vector2 size(1280, 720);
        RenderJoySettingsRequestBus::BroadcastResult(size, &RenderJoySettingsRequests::GetRenderTargetSize);
        uint32_t width = aznumeric_cast<uint32_t>(AZStd::ceilf(size.GetX()));
        uint32_t height = aznumeric_cast<uint32_t>(AZStd::ceilf(size.GetY()));

        AzToolsFramework::ViewPaneOptions options;
        options.paneRect = QRect(0, 0, width, height);
        options.isDockable = false;
        AzToolsFramework::RegisterViewPane<RenderJoy::MainWindow>(RenderJoyName, LyViewPane::CategoryOther, options);

        AzToolsFramework::RegisterViewPane<RenderJoy::RenderJoyCanvasWindow>("RenderJoyCanvas", LyViewPane::CategoryOther, options);
    }
    ////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////
    // RenderJoyCanvasSystemRequestBus Overrides
    void RenderJoyEditorSystemComponent::GetRenderJoyGraphViewConfig(AtomToolsFramework::GraphViewConfig& graphViewConfig) const
    {
        graphViewConfig = m_graphViewConfig;
    }
    ////////////////////////////////////////////////////////////////////////

} // namespace RenderJoy
