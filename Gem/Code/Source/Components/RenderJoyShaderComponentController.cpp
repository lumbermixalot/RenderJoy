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

#include <AzFramework/Entity/EntityContextBus.h>
#include <AzFramework/Entity/EntityContext.h>
#include <AzFramework/Scene/Scene.h>
#include <AzFramework/Scene/SceneSystemInterface.h>

#include <AzCore/RTTI/BehaviorContext.h>

#include <Atom/RPI.Public/Scene.h>

#include "RenderJoyShaderComponentController.h"

namespace RenderJoy
{
    void RenderJoyShaderComponentConfig::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyShaderComponentConfig>()
                ->Version(1)
                ->Field("ShaderAsset", &RenderJoyShaderComponentConfig::m_shaderAsset)
                ->Field("InputChannels", &RenderJoyShaderComponentConfig::m_inputChannels)
                ->Field("RenderTargetWidth", &RenderJoyShaderComponentConfig::m_renderTargetWidth)
                ->Field("RenderTargetHeight", &RenderJoyShaderComponentConfig::m_renderTargetHeight)
                ->Field("RenderTargetFormat", &RenderJoyShaderComponentConfig::m_outputFormat)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RenderJoyShaderComponentConfig>(
                    "RenderJoyShaderComponentConfig", "Configuration data for a RenderJoy Shader Pass.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyShaderComponentConfig::m_shaderAsset, "Shader", "The shader used in this rendering pass")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyShaderComponentConfig::m_inputChannels, "Input Channels", "The entities that provide input data for texture channels")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyShaderComponentConfig::m_renderTargetWidth, "Render Target Width", "If 0, uses the main viewport width.")
                        ->Attribute(AZ::Edit::Attributes::Min, RenderJoyShaderComponentConfig::MinRenderTargetSize)
                        ->Attribute(AZ::Edit::Attributes::Max, RenderJoyShaderComponentConfig::MaxRenderTargetSize)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyShaderComponentConfig::m_renderTargetHeight, "Render Target Height", "If 0, uses the main viewport height.")
                        ->Attribute(AZ::Edit::Attributes::Min, RenderJoyShaderComponentConfig::MinRenderTargetSize)
                        ->Attribute(AZ::Edit::Attributes::Max, RenderJoyShaderComponentConfig::MaxRenderTargetSize)
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &RenderJoyShaderComponentConfig::m_outputFormat, "Render Target Format", "Pixel format for the render target. If you choose a format without 4 color components, you'll have to change the PSOutput::m_color semantic accordingly.")
                        ->EnumAttribute(AZ::RHI::Format::R32G32B32A32_FLOAT, "R32G32B32A32_FLOAT")
                        ->EnumAttribute(AZ::RHI::Format::R32G32B32A32_UINT,  "R32G32B32A32_UINT")
                        ->EnumAttribute(AZ::RHI::Format::R32G32B32A32_SINT,  "R32G32B32A32_SINT")
                        
                        ->EnumAttribute(AZ::RHI::Format::R16G16B16A16_FLOAT, "R16G16B16A16_FLOAT")
                        ->EnumAttribute(AZ::RHI::Format::R16G16B16A16_UNORM, "R16G16B16A16_UNORM")
                        ->EnumAttribute(AZ::RHI::Format::R16G16B16A16_UINT,  "R16G16B16A16_UINT")
                        ->EnumAttribute(AZ::RHI::Format::R16G16B16A16_SNORM, "R16G16B16A16_SNORM")
                        ->EnumAttribute(AZ::RHI::Format::R16G16B16A16_SINT,  "R16G16B16A16_SINT")

                        ->EnumAttribute(AZ::RHI::Format::R10G10B10A2_UNORM, "R10G10B10A2_UNORM")
                        ->EnumAttribute(AZ::RHI::Format::R10G10B10A2_UINT,  "R10G10B10A2_UINT")

                        ->EnumAttribute(AZ::RHI::Format::R8G8B8A8_UNORM,      "R8G8B8A8_UNORM")
                        ->EnumAttribute(AZ::RHI::Format::R8G8B8A8_UNORM_SRGB, "R8G8B8A8_UNORM_SRGB")
                        ->EnumAttribute(AZ::RHI::Format::R8G8B8A8_UINT,       "R8G8B8A8_UINT")
                        ->EnumAttribute(AZ::RHI::Format::R8G8B8A8_SNORM,      "R8G8B8A8_SNORM")
                        ->EnumAttribute(AZ::RHI::Format::R8G8B8A8_SINT,       "R8G8B8A8_SINT")
                    ;
            }
        }
    }

    void RenderJoyShaderComponentController::Reflect(AZ::ReflectContext* context)
    {
        RenderJoyShaderComponentConfig::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyShaderComponentController>()
                ->Version(0)
                ->Field("Configuration", &RenderJoyShaderComponentController::m_configuration);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RenderJoyShaderComponentController>(
                    "RenderJoyShaderComponentController", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyShaderComponentController::m_configuration, "Configuration", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ;
            }
        }
    }

    void RenderJoyShaderComponentController::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("TransformService"));
    }

    void RenderJoyShaderComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("RenderJoyShaderComponentService"));
    }

    void RenderJoyShaderComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("RenderJoyShaderComponentService"));
        incompatible.push_back(AZ_CRC_CE("RenderJoyTextureComponentService"));
        incompatible.push_back(AZ_CRC_CE("RenderJoyKeyboardComponentService"));
    }

    void RenderJoyShaderComponentController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("TransformService"));
    }

    RenderJoyShaderComponentController::RenderJoyShaderComponentController(const RenderJoyShaderComponentConfig& config)
        : m_configuration(config)
    {
    }

    void RenderJoyShaderComponentController::Activate(AZ::EntityId entityId)
    {
        m_prevConfiguration = m_configuration;
        m_entityId = entityId;
        RenderJoyPassRequestBus::Handler::BusConnect(entityId);
        //RenderJoyNotificationBus::Handler::BusConnect();
    }

    void RenderJoyShaderComponentController::Deactivate()
    {
        //RenderJoyNotificationBus::Handler::BusDisconnect();
        AZ::Data::AssetBus::Handler::BusDisconnect();
        RenderJoyPassRequestBus::Handler::BusDisconnect();
    }

    void RenderJoyShaderComponentController::SetConfiguration(const RenderJoyShaderComponentConfig& config)
    {
        m_configuration = config;
        m_prevConfiguration = config;
    }

    const RenderJoyShaderComponentConfig& RenderJoyShaderComponentController::GetConfiguration() const
    {
        return m_configuration;
    }

    void RenderJoyShaderComponentController::OnConfigurationChanged()
    {
        if (m_prevConfiguration.m_shaderAsset != m_configuration.m_shaderAsset)
        {
            const auto shaderAssetId = m_configuration.m_shaderAsset.GetId();
            if (shaderAssetId.IsValid())
            {
                AZ::Data::AssetBus::Handler::BusConnect(shaderAssetId);
                m_configuration.m_shaderAsset.QueueLoad();
            }
            RenderJoyPassNotificationBus::Event(m_entityId, &RenderJoyPassNotifications::OnShaderAssetChanged, m_configuration.m_shaderAsset);
        }
        m_prevConfiguration = m_configuration;
    }

    /////////////////////////////////////////////////////////////////
    /// RenderJoyPassRequestBus::Handler overrides START
    AZStd::vector<AZ::EntityId> RenderJoyShaderComponentController::GetEntitiesOnInputChannels() const
    {
        AZStd::vector<AZ::EntityId> retList;
        retList.reserve(m_configuration.m_inputChannels.size());

        for (const auto& entityId : m_configuration.m_inputChannels)
        {
            retList.push_back(entityId);
        }

        return retList;
    }

    AZ::Data::Asset<AZ::RPI::ShaderAsset> RenderJoyShaderComponentController::GetShaderAsset() const
    {
        return m_configuration.m_shaderAsset;
    }

    uint32_t RenderJoyShaderComponentController::GetRenderTargetWidth() const
    {
        return m_configuration.m_renderTargetWidth;
    }

    uint32_t RenderJoyShaderComponentController::GetRenderTargetHeight() const
    {
        return m_configuration.m_renderTargetHeight;
    }

    AZ::RHI::Format RenderJoyShaderComponentController::GetRenderTargetFormat() const
    {
        return m_configuration.m_outputFormat;
    }
    /// RenderJoyPassRequestBus::Handler overrides END
    /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    //! Data::AssetBus START
    void RenderJoyShaderComponentController::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        NotifyShaderAssetChanged(asset);
    }

    void RenderJoyShaderComponentController::OnAssetError(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        NotifyShaderAssetChanged(asset);
    }
    //! Data::AssetBus END
    /////////////////////////////////////////////////////////////////

    void RenderJoyShaderComponentController::NotifyShaderAssetChanged(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        m_configuration.m_shaderAsset = asset;
        AZ::Data::AssetBus::Handler::BusDisconnect();
        auto notifyAssetchangedFn = [asset = asset, this]()
            {
                RenderJoyPassNotificationBus::Event(m_entityId, &RenderJoyPassNotifications::OnShaderAssetChanged, asset);
            };
        AZ::TickBus::QueueFunction(AZStd::move(notifyAssetchangedFn));
    }

    // void RenderJoyShaderComponentController::RegisterFeatureProcessorEventHandler(FeatureProcessorEvent::Handler& handler)
    // {
    //     if (handler.IsConnected())
    //     {
    //         handler.Disconnect();
    //     }
    //     handler.Connect(m_featureProcessorEvent);
    // }

    // ///////////////////////////////////////////////////////////
    // // RenderJoyNotificationBus::Handler overrides START
    // void RenderJoyShaderComponentController::OnFeatureProcessorActivated()
    // {
    //     m_featureProcessorEvent.Signal(true);
    //     //// This is the right moment to get a reference to the shader pass.
    //     //auto scenePtr = AZ::RPI::Scene::GetSceneForEntityId(GetEntityId());
    //     //// Keep a reference to the billboard pass.
    //     //auto renderJoySystem = RenderJoyInterface::Get();
    //     //auto passName = renderJoySystem->GetShaderPassName(GetEntityId());
    //     //AZ::RPI::PassFilter passFilter = AZ::RPI::PassFilter::CreateWithPassName(passName, scenePtr);
    //     //AZ::RPI::Pass* existingPass = AZ::RPI::PassSystemInterface::Get()->FindFirstPass(passFilter);
    //     //m_shaderPass = azrtti_cast<RenderJoyShaderPass*>(existingPass);
    //     //AZ_Warning(LogName, m_shaderPass != nullptr, "Won't be able to capture images from shader pass named %s.\n", passName.GetCStr());
    //     //
    //     //// Update button state UI
    //     //// Force UI refresh of the component so the "Save To Disk" button becomes
    //     //// enabled again.
    //     //AzToolsFramework::ToolsApplicationNotificationBus::Broadcast(
    //     //    &AzToolsFramework::ToolsApplicationEvents::InvalidatePropertyDisplay, AzToolsFramework::Refresh_AttributesAndValues);
    // }
    // 
    // void RenderJoyShaderComponentController::OnFeatureProcessorDeactivated()
    // {
    //     m_featureProcessorEvent.Signal(false);
    //     //m_shaderPass = nullptr;
    // }
    // // RenderJoyNotificationBus::Handler overrides END
    // ///////////////////////////////////////////////////////////

}
