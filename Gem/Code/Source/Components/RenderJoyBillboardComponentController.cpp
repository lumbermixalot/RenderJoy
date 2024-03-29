/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <AzCore/Asset/AssetManager.h>
#include <AzCore/Asset/AssetManagerBus.h>
#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include <AzFramework/Entity/EntityContextBus.h>
#include <AzFramework/Entity/EntityContext.h>
#include <AzFramework/Scene/Scene.h>
#include <AzFramework/Scene/SceneSystemInterface.h>

#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Pass/PassFilter.h>

#include <Render/RenderJoyBillboardPass.h>
//#include <RenderJoy/RenderJoyFeatureProcessorInterface.h>
#include "RenderJoyBillboardComponentController.h"

namespace RenderJoy
{
    void RenderJoyBillboardComponentConfig::BillboardOptions::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<BillboardOptions>()
                ->Version(1)
                // Billboard Options
                ->Field("AlwaysFaceCamera", &BillboardOptions::m_alwaysFaceCamera)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<BillboardOptions>(
                    "BillboardOptions", "Configuration data for the RenderJoy Billboard Component in Billboard mode.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BillboardOptions::m_alwaysFaceCamera, "Always Face Camera", "Should this billboard always face the camera?")
                    ;
            }
        }
    }


    void RenderJoyBillboardComponentConfig::FlatscreenOptions::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<FlatscreenOptions>()
                ->Version(1)
                ->Field("ScaleX", &FlatscreenOptions::m_scaleX)
                ->Field("ScaleY", &FlatscreenOptions::m_scaleY)
                ->Field("PosX", &FlatscreenOptions::m_posX)
                ->Field("PosY", &FlatscreenOptions::m_posY)
                ->Field("Width", &FlatscreenOptions::m_width)
                ->Field("Height", &FlatscreenOptions::m_height)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<FlatscreenOptions>(
                    "FlatscreenOptions", "Configuration data for the RenderJoy Billboard Component in Flatscreen mode.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyBillboardComponentConfig::FlatscreenOptions::m_scaleX, "Scale X", "Additional scale to apply in the X dimension. Typically 1.0")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyBillboardComponentConfig::FlatscreenOptions::m_scaleY, "Scale Y", "Additional scale to apply in the Y dimension. Typically 1.0")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0)
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &RenderJoyBillboardComponentConfig::FlatscreenOptions::m_posX, "Pos X", "Position in X relative to the viewport width. Typically between [0.0, 1.0]")
                        ->Attribute(AZ::Edit::Attributes::Min, -1.0)
                        ->Attribute(AZ::Edit::Attributes::Max,  1.0) 
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &RenderJoyBillboardComponentConfig::FlatscreenOptions::m_posY, "Pos Y", "Position in Y relative to the viewport height. Typically between [0.0, 1.0]")
                        ->Attribute(AZ::Edit::Attributes::Min, -1.0)
                        ->Attribute(AZ::Edit::Attributes::Max,  1.0) 
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &RenderJoyBillboardComponentConfig::FlatscreenOptions::m_width, "Width", "Output width relative to the viewport width. Typically between [0.0, 1.0]")
                        ->Attribute(AZ::Edit::Attributes::Min,  0.0)
                        ->Attribute(AZ::Edit::Attributes::Max,  1.0) 
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &RenderJoyBillboardComponentConfig::FlatscreenOptions::m_height, "Height", "Output height relative to the viewport height. Typically between [0.0, 1.0]")
                        ->Attribute(AZ::Edit::Attributes::Min,  0.0)
                        ->Attribute(AZ::Edit::Attributes::Max,  1.0) 
                    ;
            }
        }
    }

    void RenderJoyBillboardComponentConfig::Reflect(AZ::ReflectContext* context)
    {
        BillboardOptions::Reflect(context);
        FlatscreenOptions::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyBillboardComponentConfig>()
                ->Version(1)
                ->Field("ShaderEntityId", &RenderJoyBillboardComponentConfig::m_shaderEntityId)
                ->Field("DisplayMode", &RenderJoyBillboardComponentConfig::m_displayMode)
                // Billboard Options
                ->Field("BillboardOptions", &RenderJoyBillboardComponentConfig::m_billboardOptions)
                // Flatscreen Options
                ->Field("FlatscreenOptions", &RenderJoyBillboardComponentConfig::m_flatscreenOptions)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RenderJoyBillboardComponentConfig>(
                    "RenderJoyBillboardComponentConfig", "Configuration data for the RenderJoy Billboard Component.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyBillboardComponentConfig::m_shaderEntityId, "RenderJoy Shader", "Entity with a RenderJoy shader")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &RenderJoyBillboardComponentConfig::m_displayMode, "Display Mode", "How to show the Rendered Texture.")
                        ->EnumAttribute(DisplayMode::Billboard, "Billboard")
                        ->EnumAttribute(DisplayMode::Flatscreen, "Flatscreen")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyBillboardComponentConfig::m_billboardOptions, "Billboard Options", "Only applicable in Billboard mode.")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyBillboardComponentConfig::m_flatscreenOptions, "Flatscreen Options", "Only applicable in Flatscreen mode.")
                    ;
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<RenderJoyBillboardBus>("RenderJoyBillboardBus", "RenderJoy Billboard Editor And Runtime API")
                ->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::RuntimeOwn)
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                ->Attribute(AZ::Edit::Attributes::Category, "renderjoy")
                ->Event("SetFlatscreenMode", &RenderJoyBillboardRequests::SetFlatscreenMode)
                ->Event("SetBillboardMode", &RenderJoyBillboardRequests::SetBillboardMode)
                ;
        }
    }

    void RenderJoyBillboardComponentController::Reflect(AZ::ReflectContext* context)
    {
        RenderJoyBillboardComponentConfig::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyBillboardComponentController>()
                ->Version(0)
                ->Field("Configuration", &RenderJoyBillboardComponentController::m_configuration);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RenderJoyBillboardComponentController>(
                    "RenderJoyBillboardComponentController", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyBillboardComponentController::m_configuration, "Configuration", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ;
            }
        }
    }

    void RenderJoyBillboardComponentController::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("TransformService"));
    }

    void RenderJoyBillboardComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("RenderJoyBillboardService"));
    }

    void RenderJoyBillboardComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("RenderJoyBillboardService"));
    }

    void RenderJoyBillboardComponentController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("TransformService"));
    }

    RenderJoyBillboardComponentController::RenderJoyBillboardComponentController(const RenderJoyBillboardComponentConfig& config)
        : m_configuration(config)
    {
    }

    void RenderJoyBillboardComponentController::Activate(AZ::EntityId entityId)
    {
        AZ_Printf(LogName, "%s for entity=%s.\n", __FUNCTION__, entityId.ToString().c_str());

        m_entityId = entityId;
        m_prevConfiguration = m_configuration;

        AZ::TransformNotificationBus::Handler::BusConnect(m_entityId);
        RenderJoyBillboardBus::Handler::BusConnect(m_entityId);

        if (!m_configuration.m_shaderEntityId.IsValid())
        {
            // nothing to do at the moment.
            return;
        }

        RenderJoyNotificationBus::Handler::BusConnect();

        // Ask the RenderJoy system if a pass template can be created.
        auto renderJoySystem = RenderJoyInterface::Get();
        AZ_Assert(!!renderJoySystem, "Failed to find the RenderJoy system interface");
        const auto success = renderJoySystem->AddRenderJoyParentPass(m_entityId, m_configuration.m_shaderEntityId);
        AZ_Error(LogName, success, "Failed to add RenderJoy Pipeline.");

    }

    void RenderJoyBillboardComponentController::Deactivate()
    {
        AZ_Printf(LogName, "%s for entity=%s.\n", __FUNCTION__, m_entityId.ToString().c_str());

        RenderJoyBillboardBus::Handler::BusDisconnect();
        AZ::TransformNotificationBus::Handler::BusDisconnect();
        RenderJoyNotificationBus::Handler::BusDisconnect();
        
        auto renderJoySystem = RenderJoyInterface::Get();
        AZ_Assert(!!renderJoySystem, "Failed to find the RenderJoy system interface");
        renderJoySystem->RemoveRenderJoyParentPass(m_entityId);
    }

    void RenderJoyBillboardComponentController::SetConfiguration(const RenderJoyBillboardComponentConfig& config)
    {
        m_configuration = config;
        m_prevConfiguration = config;
    }

    const RenderJoyBillboardComponentConfig& RenderJoyBillboardComponentController::GetConfiguration() const
    {
        return m_configuration;
    }

    void RenderJoyBillboardComponentController::OnTransformChanged([[maybe_unused]] const AZ::Transform& local, [[maybe_unused]] const AZ::Transform& world)
    {
        // if (!m_featureProcessor)
        // {
        //     return;
        // }

        //auto featureProcessor = AZ::RPI::Scene::GetFeatureProcessorForEntity<RenderJoyFeatureProcessorInterface>(m_entityId);
        if (m_billboardPass)
        {
            m_billboardPass->SetWorldTransform(world);
        }

    }

    void RenderJoyBillboardComponentController::OnConfigurationChanged()
    {
        if (m_prevConfiguration.m_shaderEntityId != m_configuration.m_shaderEntityId)
        {
            if (!m_configuration.m_shaderEntityId.IsValid())
            {
                RenderJoyNotificationBus::Handler::BusDisconnect();
            }
            else if (!RenderJoyNotificationBus::Handler::BusIsConnected())
            {
                RenderJoyNotificationBus::Handler::BusConnect();
            }

            auto renderJoySystem = RenderJoyInterface::Get();
            AZ_Assert(!!renderJoySystem, "Failed to find the RenderJoy system interface");
            // Because the Feature Processor will be recreated, we need to invalidate
            // our reference to the Billboard pass.
            m_billboardPass = nullptr;
            renderJoySystem->AddRenderJoyParentPass(m_entityId, m_configuration.m_shaderEntityId);
        }
        else if (
            (m_prevConfiguration.m_displayMode != m_configuration.m_displayMode) ||
            (m_prevConfiguration.m_billboardOptions != m_configuration.m_billboardOptions) ||
            (m_prevConfiguration.m_flatscreenOptions != m_configuration.m_flatscreenOptions)
            )
        {
            UpdateBillboardPassShaderConstants();
        }

        m_prevConfiguration = m_configuration;
    }

    void RenderJoyBillboardComponentController::UpdateBillboardPassShaderConstants()
    {
        if (m_billboardPass)
        {
            if (m_configuration.m_displayMode == RenderJoyBillboardComponentConfig::DisplayMode::Flatscreen)
            {
                const float scaleX = m_configuration.m_flatscreenOptions.m_scaleX;
                const float scaleY = m_configuration.m_flatscreenOptions.m_scaleY;
                m_billboardPass->SetFlatscreenMode(m_configuration.m_flatscreenOptions.m_posX * scaleX
                    , m_configuration.m_flatscreenOptions.m_posY * scaleY
                    , m_configuration.m_flatscreenOptions.m_width * scaleX
                    , m_configuration.m_flatscreenOptions.m_height * scaleY);
            }
            else
            {
                m_billboardPass->SetBillboardMode(m_configuration.m_billboardOptions.m_alwaysFaceCamera);
            }
        }
    }

    ///////////////////////////////////////////////////////////
    // RenderJoyNotificationBus::Handler overrides START
    void RenderJoyBillboardComponentController::OnFeatureProcessorActivated()
    {
        if (!m_configuration.m_shaderEntityId.IsValid())
        {
            return;
        }

        auto scenePtr = AZ::RPI::Scene::GetSceneForEntityId(m_entityId);
        // Keep a reference to the billboard pass.
        auto renderJoySystem = RenderJoyInterface::Get();
        auto passName = renderJoySystem->GetBillboardPassName(m_entityId);
        AZ::RPI::PassFilter passFilter = AZ::RPI::PassFilter::CreateWithPassName(passName, scenePtr);
        AZ::RPI::Pass* existingPass = AZ::RPI::PassSystemInterface::Get()->FindFirstPass(passFilter);
        m_billboardPass = azrtti_cast<RenderJoyBillboardPass*>(existingPass);

        if (!m_billboardPass)
        {
            AZ_Error(LogName, false, "%s Failed to find RenderJoyBillboardPass as: %s", __FUNCTION__, passName.GetCStr());
            return;
        }

        
        // Update shader constant data.
        AZ::Transform transform = AZ::Transform::CreateIdentity();
        AZ::TransformBus::EventResult(transform, m_entityId, &AZ::TransformBus::Events::GetWorldTM);
        m_billboardPass->SetWorldTransform(transform);
        UpdateBillboardPassShaderConstants();
    }

    void RenderJoyBillboardComponentController::OnFeatureProcessorDeactivated()
    {
        // clear the reference to the billboard pass.
        m_billboardPass = nullptr;
    }
    // RenderJoyNotificationBus::Handler overrides END
    ///////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////
    // RenderJoyBillboardBus::Handler overrides START
    void RenderJoyBillboardComponentController::SetFlatscreenMode(float posX, float posY, float width, float height)
    {
        m_configuration.m_flatscreenOptions.m_posX = posX;
        m_configuration.m_flatscreenOptions.m_posY = posY;
        m_configuration.m_flatscreenOptions.m_width = width;
        m_configuration.m_flatscreenOptions.m_height = height;
        m_configuration.m_displayMode = RenderJoyBillboardComponentConfig::DisplayMode::Flatscreen;
        OnConfigurationChanged();
    }

    void RenderJoyBillboardComponentController::SetBillboardMode(bool alwaysFaceCamera)
    {
        m_configuration.m_billboardOptions.m_alwaysFaceCamera = alwaysFaceCamera;
        m_configuration.m_displayMode = RenderJoyBillboardComponentConfig::DisplayMode::Billboard;
        OnConfigurationChanged();
    }
    // RenderJoyBillboardBus::Handler overrides END
    ///////////////////////////////////////////////////////////
}
