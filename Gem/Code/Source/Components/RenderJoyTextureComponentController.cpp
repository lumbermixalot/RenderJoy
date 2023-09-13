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

#include "RenderJoyTextureComponentController.h"

namespace RenderJoy
{
    void RenderJoyTextureComponentConfig::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyTextureComponentConfig>()
                ->Version(1)
                ->Field("ImageAsset", &RenderJoyTextureComponentConfig::m_imageAsset)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RenderJoyTextureComponentConfig>(
                    "RenderJoyTextureComponentConfig", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyTextureComponentConfig::m_imageAsset, "Texture", "The texture used for shader sampling")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::ValuesOnly)
                    ;
            }
        }
    }

    void RenderJoyTextureComponentController::Reflect(AZ::ReflectContext* context)
    {
        RenderJoyTextureComponentConfig::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyTextureComponentController>()
                ->Version(0)
                ->Field("Configuration", &RenderJoyTextureComponentController::m_configuration);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<RenderJoyTextureComponentController>(
                    "RenderJoyTextureComponentController", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyTextureComponentController::m_configuration, "Configuration", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ;
            }
        }
    }

    void RenderJoyTextureComponentController::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("TransformService"));
    }

    void RenderJoyTextureComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("RenderJoyTextureComponentService"));
    }

    void RenderJoyTextureComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("RenderJoyTextureComponentService"));
        incompatible.push_back(AZ_CRC_CE("RenderJoyShaderComponentService"));
        incompatible.push_back(AZ_CRC_CE("RenderJoyKeyboardComponentService"));
    }

    void RenderJoyTextureComponentController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("TransformService"));
    }

    RenderJoyTextureComponentController::RenderJoyTextureComponentController(const RenderJoyTextureComponentConfig& config)
        : m_configuration(config)
    {
    }

    void RenderJoyTextureComponentController::Activate(AZ::EntityId entityId)
    {
        m_prevConfiguration = m_configuration;
        m_entityId = entityId;
        RenderJoyTextureProviderBus::Handler::BusConnect(entityId);
    }

    void RenderJoyTextureComponentController::Deactivate()
    {
        RenderJoyTextureProviderNotificationBus::Event(
            m_entityId, &RenderJoyTextureProviderNotification::OnStreamingImageAssetChanged,
            AZ::Data::Asset<AZ::RPI::StreamingImageAsset>());

        AZ::Data::AssetBus::Handler::BusDisconnect();
        RenderJoyTextureProviderBus::Handler::BusDisconnect();
    }

    void RenderJoyTextureComponentController::SetConfiguration(const RenderJoyTextureComponentConfig& config)
    {
        m_configuration = config;
        m_prevConfiguration = config;
    }

    const RenderJoyTextureComponentConfig& RenderJoyTextureComponentController::GetConfiguration() const
    {
        return m_configuration;
    }

    void RenderJoyTextureComponentController::OnConfigurationChanged()
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
    AZ::Data::Asset<AZ::RPI::StreamingImageAsset> RenderJoyTextureComponentController::GetStreamingImageAsset() const
    {
        return m_configuration.m_imageAsset;
    }
    AZ::RHI::Format RenderJoyTextureComponentController::GetPixelFormat() const
    {
        if (!m_configuration.m_imageAsset.IsReady())
        {
            AZ_Error(LogName, false, "StreamingImageAsset %s is not ready.", m_configuration.m_imageAsset.GetHint().c_str());
            return AZ::RHI::Format::Unknown;
        }
        return m_configuration.m_imageAsset->GetImageDescriptor().m_format;
    }

    AZ::RHI::Size RenderJoyTextureComponentController::GetImageSize() const
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
    void RenderJoyTextureComponentController::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        OnImageAssetLoaded(asset);
    }

    void RenderJoyTextureComponentController::OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        OnImageAssetLoaded(asset);
    }

    void RenderJoyTextureComponentController::OnAssetError(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        OnImageAssetLoaded(asset);
    }

    void RenderJoyTextureComponentController::OnAssetCanceled(const AZ::Data::AssetId assetId)
    {
        AZ_Printf(LogName, "Unfortunately assetId=%s was canceled", assetId.ToString<AZStd::string>().c_str());
    }
    //! Data::AssetBus END
    /////////////////////////////////////////////////////////////////


    void RenderJoyTextureComponentController::OnImageAssetLoaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
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
