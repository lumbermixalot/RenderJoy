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

#include <RenderJoy/IKeyboardComponentsManager.h>
#include "RenderJoyKeyboardComponentController.h"

namespace RenderJoy
{
    void RenderJoyKeyboardComponentConfig::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyKeyboardComponentConfig>()
                ->Version(1)
                ->Field("ImageAsset", &RenderJoyKeyboardComponentConfig::m_imageAsset)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RenderJoyKeyboardComponentConfig>(
                    "RenderJoyKeyboardComponentConfig", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyKeyboardComponentConfig::m_imageAsset, "Texture", "The texture used for shader sampling")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::ValuesOnly)
                    ;
            }
        }
    }

    void RenderJoyKeyboardComponentController::Reflect(AZ::ReflectContext* context)
    {
        RenderJoyKeyboardComponentConfig::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyKeyboardComponentController>()
                ->Version(0)
                ->Field("Configuration", &RenderJoyKeyboardComponentController::m_configuration);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RenderJoyKeyboardComponentController>(
                    "RenderJoyKeyboardComponentController", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyKeyboardComponentController::m_configuration, "Configuration", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ;
            }
        }
    }

    void RenderJoyKeyboardComponentController::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("TransformService"));
    }

    void RenderJoyKeyboardComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("RenderJoyKeyboardComponentService"));
    }

    void RenderJoyKeyboardComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("RenderJoyKeyboardComponentService"));
        incompatible.push_back(AZ_CRC_CE("RenderJoyShaderComponentService"));
        incompatible.push_back(AZ_CRC_CE("RenderJoyKeyboardComponentService"));
    }

    void RenderJoyKeyboardComponentController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("TransformService"));
    }

    RenderJoyKeyboardComponentController::RenderJoyKeyboardComponentController(const RenderJoyKeyboardComponentConfig& config)
        : m_configuration(config)
    {
    }

    void RenderJoyKeyboardComponentController::Activate(AZ::EntityId entityId)
    {
        m_prevConfiguration = m_configuration;
        m_entityId = entityId;
        RenderJoyTextureProviderBus::Handler::BusConnect(entityId);
    }

    void RenderJoyKeyboardComponentController::Deactivate()
    {
        RenderJoyTextureProviderNotificationBus::Event(
            m_entityId, &RenderJoyTextureProviderNotification::OnStreamingImageAssetChanged,
            AZ::Data::Asset<AZ::RPI::StreamingImageAsset>());

        AZ::Data::AssetBus::Handler::BusDisconnect();
        RenderJoyTextureProviderBus::Handler::BusDisconnect();
    }

    void RenderJoyKeyboardComponentController::SetConfiguration(const RenderJoyKeyboardComponentConfig& config)
    {
        m_configuration = config;
        m_prevConfiguration = config;
    }

    const RenderJoyKeyboardComponentConfig& RenderJoyKeyboardComponentController::GetConfiguration() const
    {
        return m_configuration;
    }

    void RenderJoyKeyboardComponentController::OnConfigurationChanged()
    {
        if (m_prevConfiguration.m_imageAsset != m_configuration.m_imageAsset)
        {
            AZ::Data::AssetBus::Handler::BusDisconnect();
            RenderJoyTextureProviderNotificationBus::Event(
                m_entityId, &RenderJoyTextureProviderNotification::OnStreamingImageAssetChanged, AZ::Data::Asset<AZ::RPI::StreamingImageAsset>());

            if (m_configuration.m_imageAsset.GetId().IsValid())
            {
                AZ::Data::AssetBus::Handler::BusConnect(m_configuration.m_imageAsset.GetId());
                m_configuration.m_imageAsset.QueueLoad();
            }
        }

        m_prevConfiguration = m_configuration;
    }

    /////////////////////////////////////////////////////////////////
    /// RenderJoyTextureProviderBus::Handler overrides START
    AZ::Data::Asset<AZ::RPI::StreamingImageAsset> RenderJoyKeyboardComponentController::GetStreamingImageAsset() const
    {
        return m_configuration.m_imageAsset;
    }
    AZ::RHI::Format RenderJoyKeyboardComponentController::GetPixelFormat() const
    {
        if (!m_configuration.m_imageAsset.IsReady())
        {
            AZ_Error(LogName, false, "StreamingImageAsset %s is not ready.", m_configuration.m_imageAsset.GetHint().c_str());
            return AZ::RHI::Format::Unknown;
        }
        return m_configuration.m_imageAsset->GetImageDescriptor().m_format;
    }

    AZ::RHI::Size RenderJoyKeyboardComponentController::GetImageSize() const
    {
        if (!m_configuration.m_imageAsset.IsReady())
        {
            AZ_Error(LogName, false, "StreamingImageAsset %s is not ready.", m_configuration.m_imageAsset.GetHint().c_str());
            return AZ::RHI::Size();
        }
        return m_configuration.m_imageAsset->GetImageDescriptor().m_size;
    }
    /// RenderJoyTextureProviderBus::Handler overrides END
    /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    //! Data::AssetBus START
    void RenderJoyKeyboardComponentController::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        OnImageAssetLoaded(asset);
    }

    void RenderJoyKeyboardComponentController::OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        OnImageAssetLoaded(asset);
    }

    void RenderJoyKeyboardComponentController::OnAssetError(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        OnImageAssetLoaded(asset);
    }

    void RenderJoyKeyboardComponentController::OnAssetCanceled(const AZ::Data::AssetId assetId)
    {
        AZ_Printf(LogName, "Unfortunately assetId=%s was canceled", assetId.ToString<AZStd::string>().c_str());
    }
    //! Data::AssetBus END
    /////////////////////////////////////////////////////////////////


    void RenderJoyKeyboardComponentController::OnImageAssetLoaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        if (!asset.IsReady())
        {
            AZ_Error(LogName, false, "Streaming image %s for entityId %s Failed to load.\n"
                , asset.GetHint().c_str(), m_entityId.ToString().c_str());
            return;
        }

        m_configuration.m_imageAsset = asset;
        AZ::TickBus::QueueFunction([this]() {
            RenderJoyTextureProviderNotificationBus::Event(m_entityId
                , &RenderJoyTextureProviderNotification::OnStreamingImageAssetChanged, m_configuration.m_imageAsset);
        });
    }

}
