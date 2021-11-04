/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Component/TickBus.h>

#include "RenderJoyTextureEditorComponent.h"

namespace RenderJoy
{
    void TextureComponentConfig::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TextureComponentConfig, AZ::ComponentConfig>()
                ->Version(1)
                ->Field("ImageAsset", &TextureComponentConfig::m_imageAsset)
                ;
        }
    }

    void RenderJoyTextureEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        TextureComponentConfig::Reflect(context);

        auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<RenderJoyTextureEditorComponent, EditorComponentBase>()
                ->Version(1)
                ->Field("Config", &RenderJoyTextureEditorComponent::m_config)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<RenderJoyTextureEditorComponent>(
                    "RenderJoy Texture", "An image to be bound to one of the texture channels of a RenderJoy Pass")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "RenderJoy")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "editor/icons/components/viewport/component_placeholder.png")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZStd::vector<AZ::Crc32>({AZ_CRC_CE("Game")}))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyTextureEditorComponent::m_config, "Configuration", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &RenderJoyTextureEditorComponent::OnConfigChanged)
                    ;

                editContext->Class<TextureComponentConfig>(
                    "TextureComponentConfig", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TextureComponentConfig::m_imageAsset, "Texture", "The texture used for sampling")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::ValuesOnly)
                    ;
            }
        }

    }

    void RenderJoyTextureEditorComponent::Activate()
    {
        RenderJoyTextureProviderBus::Handler::BusConnect(GetEntityId());
        OnConfigChanged();
    }

    void RenderJoyTextureEditorComponent::Deactivate()
    {
        RenderJoyTextureProviderNotificationBus::Event(
            GetEntityId(), &RenderJoyTextureProviderNotification::OnStreamingImageAssetChanged,
            AZ::Data::Asset<AZ::RPI::StreamingImageAsset>());
        AZ::Data::AssetBus::Handler::BusDisconnect();
        RenderJoyTextureProviderBus::Handler::BusDisconnect();
    }

    //////////////////////////////////////////////////////////////////////////
    // RenderJoyTextureProviderBus overrides
    AZ::Data::Asset<AZ::RPI::StreamingImageAsset> RenderJoyTextureEditorComponent::GetStreamingImageAsset() const
    {
        return m_config.m_imageAsset;
    }
    AZ::RHI::Format RenderJoyTextureEditorComponent::GetPixelFormat() const
    {
        if (!m_config.m_imageAsset.IsReady())
        {
            AZ_Error(LogName, false, "StreamingImageAsset %s is not ready", m_config.m_imageAsset.GetHint().c_str());
            return AZ::RHI::Format::Unknown;
        }
        return m_config.m_imageAsset->GetImageDescriptor().m_format;
    }

    AZ::RHI::Size RenderJoyTextureEditorComponent::GetImageSize() const
    {
        if (!m_config.m_imageAsset.IsReady())
        {
            AZ_Error(LogName, false, "StreamingImageAsset %s is not ready", m_config.m_imageAsset.GetHint().c_str());
            return AZ::RHI::Size();
        }
        return m_config.m_imageAsset->GetImageDescriptor().m_size;
    }
    //////////////////////////////////////////////////////////////////////////

    void RenderJoyTextureEditorComponent::OnConfigChanged()
    {
        if (!m_config.m_imageAsset.GetId().IsValid())
        {
            return;
        }
        AZ_TracePrintf(LogName, "OnConfigChanged=%s\n", m_config.m_imageAsset.GetHint().c_str());
        AZ::Data::AssetId assetId = m_config.m_imageAsset.GetId();
        m_config.m_imageAsset = {};
        RenderJoyTextureProviderNotificationBus::Event(
            GetEntityId(), &RenderJoyTextureProviderNotification::OnStreamingImageAssetChanged, AZ::Data::Asset<AZ::RPI::StreamingImageAsset>());

        AZ::TickBus::QueueFunction([this, assetId]() {
                StartLoadingImage(assetId);
        });

    }


    void RenderJoyTextureEditorComponent::StartLoadingImage(AZ::Data::AssetId assetId)
    {
        AZ::Data::AssetBus::Handler::BusDisconnect();
        if (!assetId.IsValid())
        {
            return;
        }

        // If the asset is already loaded it'll call OnAssetReady() immediately on BusConnect().
        AZ::Data::AssetBus::Handler::BusConnect(assetId);
        m_config.m_imageAsset = AZ::Data::AssetManager::Instance().GetAsset<AZ::RPI::StreamingImageAsset>(assetId, AZ::Data::AssetLoadBehavior::QueueLoad);
    }

    void RenderJoyTextureEditorComponent::OnImageAssetLoaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        AZ_TracePrintf(LogName, "Streaming image %s for entityId %s is %s\n", asset.GetHint().c_str(), GetEntityId().ToString().c_str(),
            asset.IsReady() ? "Ready" : "NOT Ready");

        if (!asset.GetId().IsValid() || !asset.IsReady())
        {
            return;
        }

        m_config.m_imageAsset = asset;
        RenderJoyTextureProviderNotificationBus::Event(GetEntityId(), &RenderJoyTextureProviderNotification::OnStreamingImageAssetChanged, m_config.m_imageAsset);

    }

    //////////////////////////////////////////////////////////////////////////////////
    //  AZ::Data::AssetBus overrides
    void RenderJoyTextureEditorComponent::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        OnImageAssetLoaded(asset);
    }

    void RenderJoyTextureEditorComponent::OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        OnImageAssetLoaded(asset);
    }

    void RenderJoyTextureEditorComponent::OnAssetError(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        OnImageAssetLoaded(asset);
    }

    void RenderJoyTextureEditorComponent::OnAssetCanceled(const AZ::Data::AssetId assetId)
    {
        AZ_TracePrintf(LogName, "Unfortunately assetId=%s was canceled", assetId.ToString<AZStd::string>().c_str());
    }
    //////////////////////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////////////

} // namespace RenderJoy
