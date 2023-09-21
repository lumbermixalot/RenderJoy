/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Component/Entity.h>
#include <AzCore/IO/SystemFile.h>

#include <AzFramework/StringFunc/StringFunc.h>

#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/Entity/EditorEntityInfoBus.h>
#include <AzToolsFramework/API/EditorAssetSystemAPI.h>
#include <AzToolsFramework/UI/PropertyEditor/PropertyFilePathCtrl.h>

#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Pass/PassFilter.h>

#include <Tools/Utils/PngTextureWriter.h>
#include <Tools/Utils/DdsTextureWriter.h>
#include <Render/RenderJoyShaderPass.h>
#include "EditorRenderJoyShaderComponent.h"

AZ_PUSH_DISABLE_WARNING(4251 4800, "-Wunknown-warning-option") // disable warnings spawned by QT
#include <QApplication.h>
#include <QMessageBox>
AZ_POP_DISABLE_WARNING

namespace RenderJoy
{
    void SaveToDiskConfig::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context);
        if (serialize)
        {
            serialize->Class<SaveToDiskConfig, AZ::ComponentConfig>()
                ->Version(1)
                ->Field("OutputImagePath", &SaveToDiskConfig::m_outputImagePath)
                ;

            AZ::EditContext* edit = serialize->GetEditContext();
            if (edit)
            {
                edit->Class<SaveToDiskConfig>("Save To Disk", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &SaveToDiskConfig::m_outputImagePath, "Output Path",
                        "Output path to save the image(s) to.")
                    ->Attribute(AZ::Edit::Attributes::SourceAssetFilterPattern, SaveToDiskConfig::GetSupportedImagesFilter())
                    ;
            }
        }
    }

    void EditorRenderJoyShaderComponent::Reflect(AZ::ReflectContext* context)
    {
        BaseClass::Reflect(context);
        SaveToDiskConfig::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorRenderJoyShaderComponent, BaseClass>()
                ->Version(1)
                ->Field("SaveToDiskConfig", &EditorRenderJoyShaderComponent::m_saveToDiskConfig)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<EditorRenderJoyShaderComponent>(
                    "RenderJoy Shader", "The RenderJoy Shader component")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Graphics/RenderJoy")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Icons/Components/Viewport/Component_Placeholder.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->Attribute(AZ::Edit::Attributes::HelpPageURL, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Save To Disk")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &EditorRenderJoyShaderComponent::m_saveToDiskConfig, "Configuration", "")
                            ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                        ->UIElement(AZ::Edit::UIHandlers::Button, "SaveToDisk", "Save the shader output as an image.")
                            ->Attribute(AZ::Edit::Attributes::NameLabelOverride, "")
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Capture & Save")
                            ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorRenderJoyShaderComponent::OnSaveToDisk)
                            ->Attribute(AZ::Edit::Attributes::ReadOnly, &EditorRenderJoyShaderComponent::IsSaveToDiskDisabled)
                    ->EndGroup()
                    ;
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->ConstantProperty("EditorRenderJoyShaderComponentTypeId", BehaviorConstant(AZ::Uuid(EditorRenderJoyShaderComponentTypeId)))
                ->Attribute(AZ::Script::Attributes::Module, "render")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Automation);
        }
    }

    EditorRenderJoyShaderComponent::EditorRenderJoyShaderComponent()
    {
    }

    EditorRenderJoyShaderComponent::EditorRenderJoyShaderComponent(const RenderJoyShaderComponentConfig& config)
        : BaseClass(config)
    {
    }

    void EditorRenderJoyShaderComponent::Activate()
    {
        BaseClass::Activate();
        AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusConnect(GetEntityId());
        AzToolsFramework::EditorEntityInfoNotificationBus::Handler::BusConnect();
        RenderJoyNotificationBus::Handler::BusConnect();
    }

    void EditorRenderJoyShaderComponent::Deactivate()
    {
        RenderJoyNotificationBus::Handler::BusDisconnect();
        AzToolsFramework::EditorEntityInfoNotificationBus::Handler::BusDisconnect();
        AzToolsFramework::EditorComponentSelectionRequestsBus::Handler::BusDisconnect();
        BaseClass::Deactivate();
    }


    void EditorRenderJoyShaderComponent::OnEntityVisibilityChanged([[maybe_unused]] bool visibility)
    {

    }

    // EditorComponentAdapter overrides
    AZ::u32 EditorRenderJoyShaderComponent::OnConfigurationChanged()
    {
        m_controller.OnConfigurationChanged();
        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    // Captures the current rendered image and saves it to disk.
    AZ::u32 EditorRenderJoyShaderComponent::OnSaveToDisk()
    {
        AZ::IO::Path fullPathIO = AzToolsFramework::GetAbsolutePathFromRelativePath(m_saveToDiskConfig.m_outputImagePath);
        if (m_saveToDiskConfig.m_outputImagePath.empty() || fullPathIO.empty())
        {
            QString msg("Invalid output path.");
            QMessageBox::information(
                QApplication::activeWindow(),
                "Error",
                msg,
                QMessageBox::Ok);
            return AZ::Edit::PropertyRefreshLevels::None;
        }
        fullPathIO = fullPathIO.LexicallyNormal();

        AZStd::string parentPath = fullPathIO.ParentPath().String();
        // Make sure the output directory exists:
        if (!AZ::IO::SystemFile::Exists(parentPath.c_str()))
        {
            QString msg = QString::asprintf("Output directory=<%s> doesn't exist!", parentPath.c_str());
            QMessageBox::information(
                QApplication::activeWindow(),
                "Error",
                msg,
                QMessageBox::Ok);
            return AZ::Edit::PropertyRefreshLevels::None;
        }

        AZStd::string prefix = fullPathIO.Stem().String();
        const uint16_t mipLevels = 1;
        const AZ::RHI::Format pixFormat = m_controller.GetRenderTargetFormat();
        m_textureWriter.reset();
        if (fullPathIO.Extension() == ".png")
        {
            m_textureWriter = AZStd::make_unique<PngTextureWriter>(
                mipLevels, pixFormat, parentPath, prefix);
        }
        else if (fullPathIO.Extension() == ".dds")
        {
            m_textureWriter = AZStd::make_unique<DdsTextureWriter>(
                mipLevels, pixFormat, parentPath, prefix);
        }
        else
        {
            QString msg = QString::asprintf("Image extension <%s> is not supported.", fullPathIO.Extension().String().c_str());
            QMessageBox::information(
                QApplication::activeWindow(),
                "Error",
                msg,
                QMessageBox::Ok);
            return AZ::Edit::PropertyRefreshLevels::None;
        }

        if (!m_attachmentReadback)
        {
            m_readbackTaskId = AZ::Crc32(GetEntityId().ToString());
            AZStd::fixed_string<128> scope_name = AZStd::fixed_string<128>::format("RenderJoyCapture_%u", m_readbackTaskId);
            m_attachmentReadback = AZStd::make_shared<AZ::RPI::AttachmentReadback>(AZ::RHI::ScopeId{ scope_name });
            m_attachmentReadback->SetCallback(AZStd::bind(&EditorRenderJoyShaderComponent::AttachmentReadbackCallback, this, AZStd::placeholders::_1));
            m_attachmentReadback->SetUserIdentifier(m_readbackTaskId);
        }

        AZ::Name slotName("Output");
        const bool result = m_shaderPass->ReadbackAttachment(m_attachmentReadback, m_readbackTaskId,
            slotName, AZ::RPI::PassAttachmentReadbackOption::Output);
        AZ_Error(LogName, result, "Failed to schedule attachment readback capture.");

        return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
    }

    // Helper function that makes the SaveToDisk button disabled or enabled.
    bool EditorRenderJoyShaderComponent::IsSaveToDiskDisabled()
    {
        return m_shaderPass == nullptr;
    }

    void EditorRenderJoyShaderComponent::AttachmentReadbackCallback(const AZ::RPI::AttachmentReadback::ReadbackResult& result)
    {
        AZ_Assert(result.m_userIdentifier == m_readbackTaskId, "Got unexpected user identifier <%u>. Was expecting <%u>.", result.m_userIdentifier, m_readbackTaskId);

        if (!m_textureWriter)
        {
            return;
        }

        const auto& mipDataBuffer = result.m_mipDataBuffers[0];
        if (!m_textureWriter->SetDataBufferForMipLevel(mipDataBuffer.m_mipBuffer, mipDataBuffer.m_mipInfo.m_slice, mipDataBuffer.m_mipInfo.m_size))
        {
            AZ_Error(LogName, false, "%s Failed to add data buffer for level=%hu.", __FUNCTION__, 0);
            m_textureWriter.reset();
            return;
        }

        // We got all the data. Save it to disk.
        AZStd::vector<AZ::IO::Path> savedFiles;
        AZStd::string msg;
        bool success = m_textureWriter->SaveMipLevel(0, &savedFiles);
        if (!success)
        {
            msg = AZStd::string::format("Failed to save texture as %s/%s"
                , m_textureWriter->GetOuputDir().String().c_str(), m_textureWriter->GetStemPrefix().c_str());
        }
        else
        {
            msg = AZStd::string::format("Successfully captured render target as %s!", savedFiles[0].String().c_str());
        }
        auto notifyAssetchangedFn = [success=success, msg=msg, this]()
            {
                OnSavingToDiskCompleted(success, msg);
            };
        AZ::TickBus::QueueFunction(AZStd::move(notifyAssetchangedFn));
    }

    void EditorRenderJoyShaderComponent::OnSavingToDiskCompleted(bool success, AZStd::string message)
    {
        QString msg(message.c_str());
        QMessageBox::information(
            QApplication::activeWindow(),
            success ? "Success" : "Error",
            msg,
            QMessageBox::Ok);

        // Force UI refresh of the component so the "Save To Disk" button becomes
        // enabled again.
        AzToolsFramework::ToolsApplicationNotificationBus::Broadcast(
            &AzToolsFramework::ToolsApplicationEvents::InvalidatePropertyDisplay, AzToolsFramework::Refresh_AttributesAndValues);

        m_textureWriter.reset();
    }

    ///////////////////////////////////////////////////////////
    // RenderJoyNotificationBus::Handler overrides START
    void EditorRenderJoyShaderComponent::OnFeatureProcessorActivated()
    {
        if (m_shaderPass != nullptr)
        {
            return;
        }

        // This is the right moment to get a reference to the shader pass.
        auto scenePtr = AZ::RPI::Scene::GetSceneForEntityId(GetEntityId());
        // Keep a reference to the billboard pass.
        auto renderJoySystem = RenderJoyInterface::Get();
        auto passName = renderJoySystem->GetShaderPassName(GetEntityId());
        AZ::RPI::PassFilter passFilter = AZ::RPI::PassFilter::CreateWithPassName(passName, scenePtr);
        AZ::RPI::Pass* existingPass = AZ::RPI::PassSystemInterface::Get()->FindFirstPass(passFilter);
        m_shaderPass = azrtti_cast<RenderJoyShaderPass*>(existingPass);
        AZ_Warning(LogName, m_shaderPass != nullptr, "Won't be able to capture images from shader pass named %s.\n", passName.GetCStr());
        
        // Update button state UI
        // Force UI refresh of the component so the "Save To Disk" button becomes
        // enabled again.
        AzToolsFramework::ToolsApplicationNotificationBus::Broadcast(
            &AzToolsFramework::ToolsApplicationEvents::InvalidatePropertyDisplay, AzToolsFramework::Refresh_AttributesAndValues);
    }

    void EditorRenderJoyShaderComponent::OnFeatureProcessorDeactivated()
    {
        m_shaderPass = nullptr;
    }
    // RenderJoyNotificationBus::Handler overrides END
    ///////////////////////////////////////////////////////////

} // namespace RenderJoy
