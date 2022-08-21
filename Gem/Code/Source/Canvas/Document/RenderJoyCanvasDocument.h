/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/RTTI/RTTI.h>
#include <GraphModel/GraphModelBus.h>
#include <GraphModel/Model/GraphContext.h>
#include <GraphModel/Model/Node.h>

#include <AtomToolsFramework/Document/AtomToolsDocument.h>
#include <AtomToolsFramework/DynamicNode/DynamicNodeConfig.h>

#include <RenderJoy/Document/RenderJoyCanvasDocumentRequestBus.h>

namespace RenderJoy
{
    class RenderJoyCanvasDocument
        : public AtomToolsFramework::AtomToolsDocument
        , public RenderJoyCanvasDocumentRequestBus::Handler
        , public GraphModelIntegration::GraphControllerNotificationBus::Handler
    {
    public:
        AZ_RTTI(RenderJoyCanvasDocument, "{61FE15E6-B763-4498-8782-791177216F4D}", AtomToolsFramework::AtomToolsDocument);
        AZ_CLASS_ALLOCATOR(RenderJoyCanvasDocument, AZ::SystemAllocator, 0);
        AZ_DISABLE_COPY_MOVE(RenderJoyCanvasDocument);

        static void Reflect(AZ::ReflectContext* context);

        RenderJoyCanvasDocument() = default;
        RenderJoyCanvasDocument(
            const AZ::Crc32& toolId,
            const AtomToolsFramework::DocumentTypeInfo& documentTypeInfo,
            AZStd::shared_ptr<GraphModel::GraphContext> graphContext);
        virtual ~RenderJoyCanvasDocument();

        // AtomToolsFramework::AtomToolsDocument overrides...
        static AtomToolsFramework::DocumentTypeInfo BuildDocumentTypeInfo();
        AtomToolsFramework::DocumentObjectInfoVector GetObjectInfo() const override;
        bool Open(const AZStd::string& loadPath) override;
        bool Save() override;
        bool SaveAsCopy(const AZStd::string& savePath) override;
        bool SaveAsChild(const AZStd::string& savePath) override;
        bool IsOpen() const override;
        bool IsModified() const override;
        bool BeginEdit() override;
        bool EndEdit() override;

        // MaterialCanvasDocumentRequestBus::Handler overrides...
        GraphCanvas::GraphId GetGraphId() const override;
        const AZStd::vector<AZStd::string>& GetGeneratedFilePaths() const override;
        AZStd::string GetGraphName() const override;
        bool CompileGraph() const override;
        void QueueCompileGraph() const override;
        bool IsCompileGraphQueued() const override;

    private:
        // AtomToolsFramework::AtomToolsDocument overrides...
        void Clear() override;
        bool ReopenRecordState() override;
        bool ReopenRestoreState() override;

        // GraphModelIntegration::GraphControllerNotificationBus::Handler overrides...
        void OnGraphModelRequestUndoPoint() override;
        void OnGraphModelTriggerUndo() override;
        void OnGraphModelTriggerRedo() override;

        void RecordGraphState();
        void RestoreGraphState(const AZStd::vector<AZ::u8>& graphState);

        void CreateGraph(GraphModel::GraphPtr graph);
        void DestroyGraph();

        // Convert the template file path into a save file path based on the document name.
        AZStd::string GetOutputPathFromTemplatePath(const AZStd::string& templatePath) const;

        // Perform a search and replace operation on all of the strings stored in a container.
        void ReplaceStringsInContainer(
            const AZStd::string& findText, const AZStd::string& replaceText, AZStd::vector<AZStd::string>& container) const;

        // Convert special slot type names, like color, into one compatible with AZSL shader code.
        AZStd::string ConvertSlotTypeToAZSL(const AZStd::string& slotTypeName) const;

        // Convert a stored slot value into a string representation that can be injected into AZSL shader code.
        AZStd::string ConvertSlotValueToAZSL(const AZStd::any& slotValue) const;

        // Collect instructions from a slot and perform substitutions based on node and slot types, names, values, and connections.
        AZStd::vector<AZStd::string> GetInstructionsFromSlot(
            GraphModel::ConstNodePtr node, const AtomToolsFramework::DynamicNodeSlotConfig& slotConfig) const;

        // Determine if instructions contained on an input node should be used as part of code generation based on node connections.
        bool ShouldUseInstructionsFromInputNode(
            GraphModel::ConstNodePtr outputNode,
            GraphModel::ConstNodePtr inputNode,
            const AZStd::vector<AZStd::string>& inputSlotNames) const;

        // Get a list of all of the graph nodes sorted in execution order based on input connections.
        AZStd::vector<GraphModel::ConstNodePtr> GetInstructionNodesInExecutionOrder(
            GraphModel::ConstNodePtr outputNode, const AZStd::vector<AZStd::string>& inputSlotNames) const;

        // Generate AZSL instructions for an output node by evaluating all of the sorted graph nodes for connections to input slots
        AZStd::vector<AZStd::string> GetInstructionsFromConnectedNodes(
            GraphModel::ConstNodePtr outputNode, const AZStd::vector<AZStd::string>& inputSlotNames) const;

        using LineGenerationFn = AZStd::function<AZStd::vector<AZStd::string>(const AZStd::string&)>;

        // Search for marked up blocks of text from a template and replace lines between them with lines provided by a function.
        void ReplaceLinesInTemplateBlock(
            const AZStd::string& blockBeginToken,
            const AZStd::string& blockEndToken,
            const LineGenerationFn& lineGenerationFn,
            AZStd::vector<AZStd::string>& templateLines) const;

        AZ::Entity* m_sceneEntity = {};
        GraphCanvas::GraphId m_graphId;
        GraphModel::GraphPtr m_graph;
        AZStd::shared_ptr<GraphModel::GraphContext> m_graphContext;
        AZStd::vector<AZ::u8> m_graphStateForUndoRedo;
        bool m_modified = {};
        mutable bool m_compileGraphQueued = {};
        mutable AZStd::vector<AZStd::string> m_generatedFiles;
    };
} // namespace RenderJoy