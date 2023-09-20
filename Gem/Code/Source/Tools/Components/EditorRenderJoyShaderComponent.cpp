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

#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Pass/PassFilter.h>

#include "EditorRenderJoyShaderComponent.h"

namespace RenderJoy
{
    void EditorRenderJoyShaderComponent::Reflect(AZ::ReflectContext* context)
    {
        BaseClass::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorRenderJoyShaderComponent, BaseClass>()
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
                        //->DataElement(AZ::Edit::UIHandlers::Default, &EditorCloudTextureComputeComponent::m_saveToDiskConfig, "Configuration", "")
                        //->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
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
        //RenderJoyNotificationBus::Handler::BusConnect();

        //AZ::u64 entityId = (AZ::u64)GetEntityId();
        //m_controller.m_configuration.m_entityId = entityId;
    }

    void EditorRenderJoyShaderComponent::Deactivate()
    {
        //RenderJoyNotificationBus::Handler::BusDisconnect();
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
        return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
    }

    // Helper function that makes the SaveToDisk button disabled or enabled.
    bool EditorRenderJoyShaderComponent::IsSaveToDiskDisabled()
    {
        return m_shaderPass == nullptr;
    }

#if 0
    ///////////////////////////////////////////////////////////
    // RenderJoyNotificationBus::Handler overrides START
    void EditorRenderJoyShaderComponent::OnFeatureProcessorActivated()
    {
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
#endif

} // namespace RenderJoy
