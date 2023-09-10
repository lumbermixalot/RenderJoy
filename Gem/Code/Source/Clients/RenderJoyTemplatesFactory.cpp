/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Component/Component.h>

#include <Atom/RPI.Edit/Common/AssetUtils.h>
#include <Atom/RPI.Public/Pass/PassSystem.h>
#include <Atom/RPI.Public/Pass/PassFilter.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>

#include <RenderJoy/RenderJoyCommon.h>
#include <RenderJoy/RenderJoyPassBus.h>

#include <Render/RenderJoyBillboardPassData.h>
#include <Render/RenderJoyShaderPassData.h>
#include <Render/RenderJoyShaderPass.h>
#include "RenderJoyTemplatesFactory.h"

namespace RenderJoy
{
    static AZStd::string GetUniqueEntityNameStr(AZ::EntityId entityId)
    {
        return entityId.ToString();
    }

    static AZStd::string GetUniqueEntityPassNameStr(const AZStd::string& passNamePrefix, AZ::EntityId entityId)
    {
        return AZStd::string::format("%s_%s", passNamePrefix.c_str(), GetUniqueEntityNameStr(entityId).c_str());
    }

    static AZStd::string GetUniqueEntityPassTemplateNameStr(const AZStd::string& passNamePrefix, AZ::EntityId entityId)
    {
        return AZStd::string::format("%s_Template", GetUniqueEntityPassNameStr(passNamePrefix, entityId).c_str());
    }

    // static AZ::RPI::AssetReference GetAssetReferenceFromPath(const AZStd::string& path)
    // {
    //     AZ::RPI::AssetReference assetReference;
    //     assetReference.m_filePath = path;
    //     assetReference.m_assetId = AZ::RPI::AssetUtils::GetAssetIdForProductPath(path.c_str());
    //     return assetReference;
    // }

    static AZStd::shared_ptr<AZ::RPI::PassTemplate> CreateBillboardPassTemplate(AZ::EntityId parentEntityId, bool useRenderJoyAttachment)
    {
        //{
        //    "Type": "JsonSerialization",
        //    "Version": 1,
        //    "ClassName": "PassAsset",
        //    "ClassData": {
        //        "PassTemplate": {
        //            "Name": "RenderJoyBillboardPassTemplate",
        //            "PassClass": "RasterPass",
        //            "Slots": [
        //                //The input attachment is optional
        //                {
        //                    "Name": "RenderJoyImage",
        //                    "SlotType": "Input",
        //                    "ScopeAttachmentUsage": "Shader",
        //                    "ShaderInputName": "m_inputTexture"
        //                },
        //                {
        //                    "Name": "ColorInputOutput",
        //                    "SlotType": "InputOutput",
        //                    "ScopeAttachmentUsage": "RenderTarget"
        //                },
        //                {
        //                    "Name": "DepthInputOutput",
        //                    "SlotType": "InputOutput",
        //                    "ScopeAttachmentUsage": "DepthStencil"
        //                }
        //            ],
        //            "PassData": {
        //                "$type": "RenderJoyBillboardPassData",
        //                "BindViewSrg": true
        //            }
        //        }
        //    }
        //}
        const auto passClassNameStr = AZStd::string("RenderJoyBillboardPass");
        const auto templateNameStr = GetUniqueEntityPassTemplateNameStr(passClassNameStr, parentEntityId);

        auto passTemplate = AZStd::make_shared<AZ::RPI::PassTemplate>();
        passTemplate->m_name = AZ::Name(templateNameStr); // "RenderJoyBillboardPassTemplate"
        passTemplate->m_passClass = AZ::Name(passClassNameStr);

        //Three Slots, 1 optional input and 2 required outputs
        if (useRenderJoyAttachment)
        {
            AZ::RPI::PassSlot passSlot;
            passSlot.m_name = AZ::Name("RenderJoyImage");
            passSlot.m_slotType = AZ::RPI::PassSlotType::Input;
            passSlot.m_scopeAttachmentUsage = AZ::RHI::ScopeAttachmentUsage::Shader;
            passSlot.m_shaderInputName = AZ::Name("m_inputTexture");
            passTemplate->AddSlot(passSlot);
        }
        {
            AZ::RPI::PassSlot passSlot;
            passSlot.m_name = AZ::Name("ColorInputOutput");
            passSlot.m_slotType = AZ::RPI::PassSlotType::InputOutput;
            passSlot.m_scopeAttachmentUsage = AZ::RHI::ScopeAttachmentUsage::RenderTarget;
            passTemplate->AddSlot(passSlot);
        }
        {
            AZ::RPI::PassSlot passSlot;
            passSlot.m_name = AZ::Name("DepthInputOutput");
            passSlot.m_slotType = AZ::RPI::PassSlotType::InputOutput;
            passSlot.m_scopeAttachmentUsage = AZ::RHI::ScopeAttachmentUsage::DepthStencil;
            passTemplate->AddSlot(passSlot);
        }

        //PassData
        auto passData = AZStd::make_shared<RenderJoyBillboardPassData>();
        passData->m_bindViewSrg = true;
        passTemplate->m_passData = passData;

        return passTemplate;
    }

    static AZStd::shared_ptr<AZ::RPI::PassTemplate> CreateInvalidRenderJoyParentPassTemplate(AZ::RPI::PassSystemInterface* passSystem
        , AZ::EntityId parentEntityId
        , AZStd::vector<AZ::Name>& createdPassTemplateNamesOut, AZ::Name& billboardPassNameOut)
    {
        constexpr bool useRendeJoyAttachment = false;
        auto billboardPassTemplate = CreateBillboardPassTemplate(parentEntityId, useRendeJoyAttachment);

        passSystem->AddPassTemplate(billboardPassTemplate->m_name, billboardPassTemplate);
        createdPassTemplateNamesOut.push_back(billboardPassTemplate->m_name);

        //{
        //    "Type": "JsonSerialization",
        //    "Version": 1,
        //    "ClassName": "PassAsset",
        //    "ClassData": {
        //        "PassTemplate": {
        //            "Name": @name,
        //            "PassClass": "ParentPass",
        //            "Slots": [
        //                {
        //                    "Name": "AtomColorInputOutput",
        //                    "SlotType": "InputOutput",
        //                },
        //                {
        //                    "Name": "AtomDepthInputOutput",
        //                    "SlotType": "InputOutput",
        //                }
        //            ],
        //            "PassRequests": [
        //                {
        //                    "Name": "RenderJoyBillboardPass",
        //                    "TemplateName": "RenderJoyBillboardPassTemplate",
        //                    "Connections": [
        //                        {
        //                            "LocalSlot": "ColorInputOutput",
        //                            "AttachmentRef": {
        //                                "Pass": "This",
        //                                "Attachment": "AtomColorInputOutput"
        //                            }
        //                        },
        //                        {
        //                            "LocalSlot": "DepthInputOutput",
        //                            "AttachmentRef": {
        //                                "Pass": "This",
        //                                "Attachment": "AtomDepthInputOutput"
        //                            }
        //                        }
        //                    ]
        //                },
        //                ...
        //            ],
        //        }
        //    }
        //}
        const auto templateNameStr = GetUniqueEntityPassTemplateNameStr("RenderJoyParentPass", parentEntityId);

        auto passTemplate = AZStd::make_shared<AZ::RPI::PassTemplate>();
        passTemplate->m_name = AZ::Name(templateNameStr);
        passTemplate->m_passClass = AZ::Name("ParentPass");

        // Two Slots
        {
            AZ::RPI::PassSlot passSlot;
            passSlot.m_name = AZ::Name("AtomColorInputOutput");
            passSlot.m_slotType = AZ::RPI::PassSlotType::InputOutput;
            passTemplate->AddSlot(passSlot);
        }
        {
            AZ::RPI::PassSlot passSlot;
            passSlot.m_name = AZ::Name("AtomDepthInputOutput");
            passSlot.m_slotType = AZ::RPI::PassSlotType::InputOutput;
            passTemplate->AddSlot(passSlot);
        }

        // Pass Request.
        // This parent pass only has a child, which is the Billboard pass.
        {
            AZ::RPI::PassRequest childPassRequest;

            const auto billboardPassClassNameStr = AZStd::string(billboardPassTemplate->m_passClass.GetCStr());
            const auto billboardPassNameStr = GetUniqueEntityPassNameStr(billboardPassClassNameStr, parentEntityId);
            childPassRequest.m_passName = AZ::Name(billboardPassNameStr);
            childPassRequest.m_templateName = billboardPassTemplate->m_name;

            AZ::RPI::PassConnection inputConnection;
            inputConnection.m_localSlot = AZ::Name("ColorInputOutput");
            inputConnection.m_attachmentRef.m_pass = AZ::Name("Parent");
            inputConnection.m_attachmentRef.m_attachment = AZ::Name("AtomColorInputOutput");
            childPassRequest.AddInputConnection(inputConnection);

            AZ::RPI::PassConnection outputConnection;
            outputConnection.m_localSlot = AZ::Name("DepthInputOutput");
            outputConnection.m_attachmentRef.m_pass = AZ::Name("Parent");
            outputConnection.m_attachmentRef.m_attachment = AZ::Name("AtomDepthInputOutput");
            childPassRequest.AddInputConnection(outputConnection);

            passTemplate->AddPassRequest(childPassRequest);

            billboardPassNameOut = childPassRequest.m_passName;
        }

        return passTemplate;
    }

    static bool CreateInvalidRenderJoyParentPassRequest(AZ::RPI::PassSystemInterface* passSystem
        , AZ::EntityId parentEntityId
        , RenderJoyTemplatesFactory::ParentEntityTemplates& parentEntityTemplates)
    {
        auto parentPassTemplate = CreateInvalidRenderJoyParentPassTemplate(passSystem, parentEntityId
            , parentEntityTemplates.m_createdPassTemplates, parentEntityTemplates.m_billboardPassName);
        passSystem->AddPassTemplate(parentPassTemplate->m_name, parentPassTemplate);
        parentEntityTemplates.m_createdPassTemplates.push_back(parentPassTemplate->m_name);

        //{
        //    "Type": "JsonSerialization",
        //    "Version": 1,
        //    "ClassName": "PassRequest",
        //    "ClassData": {
        //        "Name": @name,
        //        "TemplateName": @name,
        //        "Enabled": true,
        //        "Connections": [
        //            {
        //                "LocalSlot": "AtomColorInputOutput",
        //                "AttachmentRef": {
        //                    "Pass": "PostProcessPass",
        //                    "Attachment": "Output"
        //                }
        //            },
        //            {
        //                "LocalSlot": "AtomDepthInputOutput",
        //                "AttachmentRef": {
        //                    "Pass": "DepthPrePass",
        //                    "Attachment": "Depth"
        //                }
        //            }
        //        ]
        //    }
        //}
        const auto passNameStr = GetUniqueEntityPassNameStr("RenderJoyParentPass", parentEntityId);

        auto passRequest = AZStd::make_shared<AZ::RPI::PassRequest>();
        passRequest->m_passName = AZ::Name(passNameStr);
        passRequest->m_templateName = parentPassTemplate->m_name;

        AZ::RPI::PassConnection inputConnection;
        inputConnection.m_localSlot = AZ::Name("AtomColorInputOutput");
        inputConnection.m_attachmentRef.m_pass = AZ::Name("PostProcessPass");
        inputConnection.m_attachmentRef.m_attachment = AZ::Name("Output");
        passRequest->AddInputConnection(inputConnection);

        AZ::RPI::PassConnection outputConnection;
        outputConnection.m_localSlot = AZ::Name("AtomDepthInputOutput");
        outputConnection.m_attachmentRef.m_pass = AZ::Name("DepthPrePass");
        outputConnection.m_attachmentRef.m_attachment = AZ::Name("Depth");
        passRequest->AddInputConnection(outputConnection);

        parentEntityTemplates.m_passRequest = passRequest;

        return true;
    }

    //! Returns true if the entity is a render joy pass (Contains a component that implements RenderJoyPassRequests)
    static bool IsRenderJoyPass(AZ::EntityId entityId)
    {
        if (!entityId.IsValid())
        {
            return false;
        }
    
        bool result = false;
        RenderJoyPassRequestBus::EnumerateHandlersId(entityId, [&](RenderJoyPassRequests* /*renderJoyPassRequest*/) -> bool
        {
            result = true;
            return true; // We expect only one handler anyways.
        });
    
        return result;
    }

    static void GetRenderJoyTargetSize(AZ::EntityId currentPassEntity, uint32_t& widthOut, uint32_t& heightOut)
    {
        widthOut = 0; heightOut = 0;
        RenderJoyPassRequestBus::EventResult(widthOut, currentPassEntity, &RenderJoyPassRequestBus::Handler::GetRenderTargetWidth);
        RenderJoyPassRequestBus::EventResult(heightOut, currentPassEntity, &RenderJoyPassRequestBus::Handler::GetRenderTargetHeight);

        if (!widthOut || !heightOut)
        {
            // Define width and height from the viewport size.
            auto viewportContextInterface = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
            const auto scenePtr = AZ::RPI::Scene::GetSceneForEntityId(currentPassEntity);
            auto viewportContext = viewportContextInterface->GetViewportContextByScene(scenePtr);
            const auto viewportSize = viewportContext->GetViewportSize();
            widthOut = viewportSize.m_width;
            heightOut = viewportSize.m_height;
        }
    }

    static bool CreateRenderJoyPassTemplatesRecursive(AZ::EntityId currentPassEntity
        , AZStd::map<AZ::EntityId, AZStd::shared_ptr<AZ::RPI::PassTemplate>>& passTemplatesOut)
    {
        static constexpr auto LogName = RenderJoyTemplatesFactory::LogName;

        // Make sure the pass has a valid shader asset.
        AZ::Data::Asset<AZ::RPI::ShaderAsset> shaderAsset;
        RenderJoyPassRequestBus::EventResult(shaderAsset, currentPassEntity, &RenderJoyPassRequests::GetShaderAsset);
        if (!shaderAsset.GetId().IsValid())
        {
            AZ_Error(LogName, false, "Failed to create pass template because the entity %s is missing the shader asset reference",
                currentPassEntity.ToString().c_str());
            return false;
        }


        AZStd::vector<AZ::EntityId> entitiesOnInputChannels;
        RenderJoyPassRequestBus::EventResult(entitiesOnInputChannels, currentPassEntity, &RenderJoyPassRequests::GetEntitiesOnInputChannels);
        for (const auto& entityId : entitiesOnInputChannels)
        {
            if (IsRenderJoyPass(entityId) && (currentPassEntity != entityId))
            {
                if (!CreateRenderJoyPassTemplatesRecursive(entityId, passTemplatesOut))
                {
                    return false;
                }
            }
        }

        const char passClassStr[] = "RenderJoyShaderPass";
        const auto PassTemplateNameStr = GetUniqueEntityPassTemplateNameStr(passClassStr, currentPassEntity);
        const auto newTemplateName = AZ::Name(PassTemplateNameStr);
    
        auto passTemplate = AZStd::make_shared<AZ::RPI::PassTemplate>();
        passTemplate->m_name = newTemplateName;
        passTemplate->m_passClass = AZ::Name(passClassStr);
    
        ////////////////////////////////////////////////////
        // Slots
        // - Input slots
        //AZ::u32 channelIndexPrevFrameOutputAsInput = RenderJoyShaderPass::InvalidInputChannelIndex;
        AZ::u32 channelIndex = 0;
        for (const auto& entityId : entitiesOnInputChannels)
        {
            //if (m_outputPassEntity == entityId)
            //{
            //    AZ_Error(
            //        LogName, false,
            //        "Failed to create pass template [%s] because the Output Pass entity with name [%s] is referring itself at channel [%u], "
            //        "which is only allowed for non-output passes",
            //        newTemplateName.GetCStr(), GetPassNameFromEntityName(currentPassEntity).c_str(), channelIndex);
            //    return false;
            //}

            AZ_Assert(currentPassEntity != entityId, "Recursive output attachments not supported yet.");
            
            //if ((currentPassEntity == entityId) || IsRenderJoyPass(entityId))
            if (IsRenderJoyPass(entityId))
            {
                AZStd::string slotNameStr = AZStd::string::format("Input%u", channelIndex);
                AZ::RPI::PassSlot inputSlot;
                inputSlot.m_name = AZ::Name(slotNameStr);
                inputSlot.m_slotType = AZ::RPI::PassSlotType::Input;
                inputSlot.m_scopeAttachmentUsage = AZ::RHI::ScopeAttachmentUsage::Shader;
                inputSlot.m_shaderInputName = AZ::Name("m_channel");
                inputSlot.m_shaderInputArrayIndex = aznumeric_caster(channelIndex);
                passTemplate->AddSlot(inputSlot);
            }
            ++channelIndex;
        }
    
        // - Output slot
        AZ::RPI::PassSlot outputSlot;
    
        outputSlot.m_name = AZ::Name("Output");
        outputSlot.m_slotType = AZ::RPI::PassSlotType::Output;
        outputSlot.m_scopeAttachmentUsage = AZ::RHI::ScopeAttachmentUsage::RenderTarget;
        outputSlot.m_loadStoreAction.m_clearValue = AZ::RHI::ClearValue::CreateVector4Float(0.0f, 0.0f, 0.0f, 0.0f);
        outputSlot.m_loadStoreAction.m_loadAction = AZ::RHI::AttachmentLoadAction::Clear;
        passTemplate->AddSlot(outputSlot);
        /////////////////////////////////////////////////////
    
        uint32_t renderTargetWidth = 0; uint32_t renderTargetHeight = 0;
        GetRenderJoyTargetSize(currentPassEntity, renderTargetWidth, renderTargetHeight);

        // Attachments:
    
        // - define the output target as transient attachment.
        // Non-output passes, aka BufferA...D in ShaderToy have R32G32B32A32_FLOAT
        // render target format
        AZ::RPI::PassImageAttachmentDesc transientAttachmentDesc;
        transientAttachmentDesc.m_name = AZ::Name("OutputAttachment");
        transientAttachmentDesc.m_imageDescriptor.m_format = AZ::RHI::Format::R32G32B32A32_FLOAT;
        transientAttachmentDesc.m_imageDescriptor.m_size.m_width = renderTargetWidth;
        transientAttachmentDesc.m_imageDescriptor.m_size.m_height = renderTargetHeight;
        passTemplate->AddImageAttachment(transientAttachmentDesc);
    
        // In case we need the output of the previous frame to become an input image
        // we need to specify such attachment.
        // REMARK1: This attachment is here specified as transient, but at runtime we'll change it to persistent.
        // REMARK2: The output pass doesn't support recursive references to itself on input channels,
        //          only non-output passes do... Just like in ShaderToy.
        // if (channelIndexPrevFrameOutputAsInput != RenderJoyTrianglePass::InvalidInputChannelIndex)
        // {
        //     AZ::RPI::PassImageAttachmentDesc transientAttachmentDesc;
        //     transientAttachmentDesc.m_name = AZ::Name("PreviousFrameImage");
        //     transientAttachmentDesc.m_imageDescriptor.m_format = AZ::RHI::Format::R32G32B32A32_FLOAT;
        //     if (renderTargetWidth && renderTargetHeight)
        //     {
        //         transientAttachmentDesc.m_imageDescriptor.m_size.m_width = renderTargetWidth;
        //         transientAttachmentDesc.m_imageDescriptor.m_size.m_height = renderTargetHeight;
        //     }
        //     else
        //     {
        //         transientAttachmentDesc.m_sizeSource.m_source.m_pass = AZ::Name("Parent");
        //         transientAttachmentDesc.m_sizeSource.m_source.m_attachment = AZ::Name("PipelineOutput");
        //     }
        //     passTemplate->AddImageAttachment(transientAttachmentDesc);
        // }
    
        // - Connections:
        // if (channelIndexPrevFrameOutputAsInput != RenderJoyTrianglePass::InvalidInputChannelIndex)
        // {
        //     AZ::RPI::PassConnection connection;
        //     connection.m_localSlot = AZ::Name(AZStd::string::format("Input%u", channelIndexPrevFrameOutputAsInput));
        //     connection.m_attachmentRef.m_pass = AZ::Name("This");
        //     connection.m_attachmentRef.m_attachment = AZ::Name("PreviousFrameImage");
        //     passTemplate->AddOutputConnection(connection);
        // }
    
        //Connect the local "Output" slot with the transient attachment.
        AZ::RPI::PassConnection connection;
        connection.m_localSlot = AZ::Name("Output");
        connection.m_attachmentRef.m_pass = AZ::Name("This");
        connection.m_attachmentRef.m_attachment = AZ::Name("OutputAttachment");
        passTemplate->AddOutputConnection(connection);
    
        // - PassData.
        auto passData = AZStd::make_shared<RenderJoyShaderPassData>();
        passData->m_shaderAsset.m_filePath = AZ::RPI::AssetUtils::GetSourcePathByAssetId(shaderAsset.GetId());
        passData->m_shaderAsset.m_assetId = shaderAsset.GetId();
        passData->m_stencilRef = 1;
        passData->m_bindViewSrg = false;
        passTemplate->m_passData = passData;
    
        passTemplatesOut.emplace(currentPassEntity, passTemplate);
        return true;
    }

    static bool CreateRenderJoyPassRequestsRecursive(AZ::RPI::PassTemplate& parentPassTemplate
        , AZ::EntityId currentPassEntity
        , const AZStd::map<AZ::EntityId, AZStd::shared_ptr<AZ::RPI::PassTemplate>>& passTemplatesDB)
    {
        AZStd::vector<AZ::EntityId> entitiesOnInputChannels;
        RenderJoyPassRequestBus::EventResult(
            entitiesOnInputChannels, currentPassEntity, &RenderJoyPassRequests::GetEntitiesOnInputChannels);
        for (const auto& entityId : entitiesOnInputChannels)
        {
            AZ_Assert(currentPassEntity != entityId, "Recursive attachments not supported yet!");

            if (IsRenderJoyPass(entityId))
            {
                if (!CreateRenderJoyPassRequestsRecursive(parentPassTemplate, entityId, passTemplatesDB))
                {
                    return false;
                }
            }
        }

        AZ_Assert(passTemplatesDB.contains(currentPassEntity), "Where is the template for entityId %s??", currentPassEntity.ToString().c_str());
        const auto& passTemplate = passTemplatesDB[currentPassEntity];

        const char passClassStr[] = "RenderJoyShaderPass";
        const auto PassNameStr = GetUniqueEntityPassNameStr(passClassStr, currentPassEntity);
        AZ::RPI::PassRequest passRequest;
        passRequest.m_passName = AZ::Name(PassNameStr);
        passRequest.m_templateName = passTemplate->m_name;
    
        // Define the connections to other RenderJoy passes.
        AZ::u32 channelIndex = 0;
        for (const auto& entityId : entitiesOnInputChannels)
        {
            if (IsRenderJoyPass(entityId))
            {
                auto slotName = AZ::Name(AZStd::string::format("Input%u", channelIndex));
                AZ::RPI::PassConnection inputConnection;
                inputConnection.m_localSlot = slotName;
                inputConnection.m_attachmentRef.m_pass = AZ::Name(GetUniqueEntityPassNameStr(passClassStr, entityId));
                inputConnection.m_attachmentRef.m_attachment = AZ::Name("Output");
                passRequest.AddInputConnection(inputConnection);
            }
            ++channelIndex;
        }
        parentPassTemplate.AddPassRequest(passRequest);
        return true;
    }

    RenderJoyTemplatesFactory::~RenderJoyTemplatesFactory()
    {
        AZ_Assert(m_parentEntities.empty(), "Do not forget to call RemoveAllTemplates");
    }


    bool RenderJoyTemplatesFactory::CreateRenderJoyParentPassRequest(AZ::RPI::PassSystemInterface* passSystem, AZ::EntityId parentPassEntityId, [[maybe_unused]] AZ::EntityId passBusEntity)
    {
        // Attempt to remove any previously created pass template.
        RemoveTemplates(passSystem, parentPassEntityId);
        
        ParentEntityTemplates parentEntityTemplates = {};
        m_parentEntities.emplace(parentPassEntityId, parentEntityTemplates);
        ParentEntityTemplates& structRef = m_parentEntities.at(parentPassEntityId);

        // Does passBusEntity implements RenderJoyPassBus?
        if (!IsRenderJoyPass(passBusEntity))
        {
            return CreateInvalidRenderJoyParentPassRequest(passSystem, parentPassEntityId, structRef);
        }

        AZStd::map<AZ::EntityId, AZStd::shared_ptr<AZ::RPI::PassTemplate>> passTemplates;
        if (!CreateRenderJoyPassTemplatesRecursive(passBusEntity, passTemplates))
        {
            return CreateInvalidRenderJoyParentPassRequest(passSystem, parentPassEntityId, structRef);
        }


        AZ::RPI::PassTemplate parentPassTemplate; //FIXME!

        // Time to create the passRequests.
        if (!CreateRenderJoyPassRequestsRecursive(parentPassTemplate, passBusEntity, passTemplates)
        {
            return CreateInvalidRenderJoyParentPassRequest(passSystem, parentPassEntityId, structRef);
        }

        // Don't forget to add all templates to the PassSystem !!
        // TODO
        
    }

    void RenderJoyTemplatesFactory::RemoveTemplates(AZ::RPI::PassSystemInterface* passSystem, AZ::EntityId parentPassEntityId)
    {
        auto itor = m_parentEntities.find(parentPassEntityId);
        if (itor == m_parentEntities.end())
        {
            return;
        }
        for (const auto& templateName : itor->second.m_createdPassTemplates)
        {
            passSystem->RemovePassTemplate(templateName);
        }
        itor->second.m_createdPassTemplates.clear();
        m_parentEntities.erase(itor);
    }

    // Wholesale removes all RenderJoy related templates from the pass system.
    void RenderJoyTemplatesFactory::RemoveAllTemplates(AZ::RPI::PassSystemInterface* passSystem)
    {
        const auto parentEntities = GetParentPassEntities();
        for (const auto &entityId : parentEntities)
        {
            RemoveTemplates(passSystem, entityId);
        }
    }

    size_t RenderJoyTemplatesFactory::GetParentPassCount() const
    {
        return m_parentEntities.size();
    }

    AZStd::vector<AZ::EntityId> RenderJoyTemplatesFactory::GetParentPassEntities() const
    {
        AZStd::vector<AZ::EntityId> retList;
        for (const auto &itor : m_parentEntities)
        {
            retList.push_back(itor.first);
        }
        return retList;
    }

    AZStd::shared_ptr<AZ::RPI::PassRequest> RenderJoyTemplatesFactory::GetParentPassRequest(AZ::EntityId parentPassEntityId) const
    {
        const auto itor = m_parentEntities.find(parentPassEntityId);
        return (itor != m_parentEntities.end()) ? itor->second.m_passRequest : nullptr;
    }

    AZ::Name RenderJoyTemplatesFactory::GetBillboardPassName(AZ::EntityId parentPassEntityId) const
    {
        const auto itor = m_parentEntities.find(parentPassEntityId);
        return (itor != m_parentEntities.end()) ? itor->second.m_billboardPassName : AZ::Name();
    }

    bool RenderJoyTemplatesFactory::EntityHasActivePasses(AZ::RPI::PassSystemInterface* passSystem, AZ::EntityId parentPassEntityId) const
    {
        if (!m_parentEntities.contains(parentPassEntityId))
        {
            return false;
        }

        const ParentEntityTemplates& templatesData = m_parentEntities.at(parentPassEntityId);
        const auto& parentPassName = templatesData.m_passRequest->m_passName;

        const auto scenePtr = AZ::RPI::Scene::GetSceneForEntityId(parentPassEntityId);
        AZ::RPI::PassFilter passFilter = AZ::RPI::PassFilter::CreateWithPassName(parentPassName, scenePtr);
        AZ::RPI::Pass* existingPass = passSystem->FindFirstPass(passFilter);
        return existingPass != nullptr;
    }

    // bool RenderJoyTemplatesFactory::Create(AZ::RPI::PassSystemInterface* passSystem)
    // {
    //     AZ::u32 outputPassCount = 0;
    //     AZ::EntityId theOutputPassEntity = RenderJoyPassRequestBusUtils::DiscoverOutputPass(outputPassCount);
    //     if (theOutputPassEntity.IsValid() && (outputPassCount == 1))
    //     {
    //         AZ_TracePrintf(LogName, "Valid: Found %u output pass entities\n", outputPassCount);
    //     }
    //     else
    //     {
    //         AZ_TracePrintf(LogName, "Invalid: Found %u output pass entities\n", outputPassCount);
    //         return false;
    //     }
    // 
    //     m_outputPassEntity = theOutputPassEntity;
    // 
    //     // Save the name of the template for the output pass
    //     auto renderJoyTrianglePassClassName = AZ::Name("RenderJoyTrianglePass");
    //     if (!passSystem->HasCreatorForClass(renderJoyTrianglePassClassName))
    //     {
    //         passSystem->AddPassCreator(renderJoyTrianglePassClassName, &RenderJoyTrianglePass::Create);
    //     }
    // 
    //     m_outputPassTemplateName = GetPassTemplateNameFromEntityName(theOutputPassEntity);
    // 
    //     if (!CreateRenderJoyPassTemplates(passSystem, theOutputPassEntity))
    //     {
    //         AZ_Error(LogName, false, "Failed to create all RenderJoy pass templates");
    //         return false;
    //     }
    // 
    //     auto passTemplate = AZStd::make_shared<AZ::RPI::PassTemplate>();
    //     passTemplate->m_name = AZ::Name(PassTemplateName);
    //     passTemplate->m_passClass = AZ::Name("ParentPass");
    // 
    //     // Slots (only one)
    //     AZ::RPI::PassSlot passSlot;
    //     passSlot.m_name = AZ::Name("PipelineOutput");
    //     passSlot.m_slotType = AZ::RPI::PassSlotType::InputOutput;
    //     passSlot.m_scopeAttachmentUsage = AZ::RHI::ScopeAttachmentUsage::RenderTarget;
    //     passTemplate->AddSlot(passSlot);
    // 
    //     // PassRequests
    //     m_outputPassName = AZ::Name(GetPassNameFromEntityName(theOutputPassEntity));
    //     if (!CreateRenderJoyPassRequests(*passTemplate.get(), theOutputPassEntity))
    //     {
    //         AZ_Error(LogName, false, "Failed to create all RenderJoy pass requests");
    //         return false;
    //     }
    //     {
    //         AZ::RPI::PassRequest passRequest;
    //         passRequest.m_passName = AZ::Name("CopyToSwapChain");
    //         passRequest.m_templateName = AZ::Name("FullscreenCopyTemplate");
    // 
    //         AZ::RPI::PassConnection inputConnection;
    //         inputConnection.m_localSlot = AZ::Name("Input");
    //         inputConnection.m_attachmentRef.m_pass = m_outputPassName;
    //         inputConnection.m_attachmentRef.m_attachment = AZ::Name("Output");
    //         passRequest.AddInputConnection(inputConnection);
    // 
    //         AZ::RPI::PassConnection outputConnection;
    //         outputConnection.m_localSlot = AZ::Name("Output");
    //         outputConnection.m_attachmentRef.m_pass = AZ::Name("Parent");
    //         outputConnection.m_attachmentRef.m_attachment = passSlot.m_name;
    //         passRequest.AddInputConnection(outputConnection);
    // 
    //         passTemplate->AddPassRequest(passRequest);
    //     }
    // 
    //     AZ::Name passTemplateName = AZ::Name("RenderJoyMainPassTemplate");
    //     m_createdPassTemplates.push_back(passTemplateName);
    //     return passSystem->AddPassTemplate(passTemplateName, passTemplate);
    // }
    // 

    // 
    // 
    // AZStd::string RenderJoyTemplatesFactory::GetPassNameFromEntityName(AZ::EntityId entityId)
    // {
    //     return AzToolsFramework::GetEntityName(entityId);
    // }
    // 
    // AZStd::string RenderJoyTemplatesFactory::GetPassTemplateNameFromEntityName(AZ::EntityId entityId)
    // {
    //     return GetPassNameFromEntityName(entityId) + "Template";
    // }
    // 
    // void RenderJoyTemplatesFactory::RemoveTemplates(AZ::RPI::PassSystemInterface* passSystem)
    // {
    //     for (const auto &templateName : m_createdPassTemplates)
    //     {
    //         passSystem->RemovePassTemplate(templateName);
    //     }
    //     m_createdPassTemplates.clear();
    // }
    // 
    // namespace RenderJoyPassRequestBusUtils
    // {
    //     //! enumerates all render joy passes and it will return the last entity
    //     //! that claims to be the output pass.
    //     AZ::EntityId DiscoverOutputPass(AZ::u32& outputPassCount)
    //     {
    //         outputPassCount = 0;
    //         AZ::EntityId theOutputPass;
    //         RenderJoyPassRequestBus::EnumerateHandlers([&](RenderJoyPassRequests* renderJoyPassRequest) -> bool
    //         {
    //             if (renderJoyPassRequest->IsOutputPass())
    //             {
    //                 if (auto* component = azrtti_cast<AZ::Component*>(renderJoyPassRequest))
    //                 {
    //                     theOutputPass = component->GetEntityId();
    //                 }
    //                 ++outputPassCount;
    //             }
    //             return true; // We must loop through all handlers.
    //         });
    //         return theOutputPass;
    //     }
    // 

    // 
    //     bool IsRenderJoyTextureProvider(AZ::EntityId entityId)
    //     {
    //         if (!entityId.IsValid())
    //         {
    //             return false;
    //         }
    // 
    //         bool result = false;
    //         RenderJoyTextureProviderBus::EnumerateHandlersId(entityId, [&](RenderJoyTextureProvider * /*renderJoyPassRequest*/) -> bool {
    //             result = true;
    //             return true; // We expect only one handler anyways.
    //         });
    // 
    //         return result;
    //     }
    // 
    //     AZ::EntityId GetEntityIdFromPassName(const AZ::Name& passName)
    //     {
    //         const AZStd::string entityName = passName.GetStringView();
    //         AZ::EntityId entityMatch;
    //         RenderJoyPassRequestBus::EnumerateHandlers([&](RenderJoyPassRequests* renderJoyPassRequest) -> bool {
    //             if (auto* component = azrtti_cast<AZ::Component*>(renderJoyPassRequest))
    //             {
    //                 auto entityId = component->GetEntityId();
    //                 if (entityId.IsValid())
    //                 {
    //                     auto namedEntityId = component->GetNamedEntityId();
    //                     if (namedEntityId.GetName() == entityName)
    //                     {
    //                         entityMatch = entityId;
    //                         return false; //Found it. Stop here.
    //                     }
    //                 }
    //             }
    //             return true; // We must loop through all handlers.
    //         });
    //         return entityMatch;
    //     }
    // 
    // } // namespace RenderJoyPassRequestBusUtils

} // namespace RenderJoy
