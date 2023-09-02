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

#include "RenderJoyPassEditorComponent.h"

namespace RenderJoy
{
    void PassComponentConfig::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<PassComponentConfig, AZ::ComponentConfig>()
                ->Version(1)
                ->Field("ShaderAsset", &PassComponentConfig::m_shaderAsset)
                ->Field("InputChannels", &PassComponentConfig::m_inputChannels)
                ->Field("IsOutputPass", &PassComponentConfig::m_isOutputPass)
                ->Field("RenderTargetWidth", &PassComponentConfig::m_renderTargetWidth)
                ->Field("RenderTargetHeight", &PassComponentConfig::m_renderTargetHeight)
                ;
        }
    }

    void RenderJoyPassEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        PassComponentConfig::Reflect(context);

        auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<RenderJoyPassEditorComponent, EditorComponentBase>()
                ->Version(1)
                ->Field("Config", &RenderJoyPassEditorComponent::m_config)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<RenderJoyPassEditorComponent>(
                    "RenderJoy Pass", "A shader that works as RenderJoy pass")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "RenderJoy")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "editor/icons/components/viewport/component_placeholder.png")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZStd::vector<AZ::Crc32>({ AZ_CRC("Game", 0x232b318c) }))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyPassEditorComponent::m_config, "Configuration", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &RenderJoyPassEditorComponent::OnConfigChanged)
                    ;

                editContext->Class<PassComponentConfig>(
                    "PassComponentConfig", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &PassComponentConfig::m_shaderAsset, "Shader", "The shader used in this rendering pass")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::ValuesOnly)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &PassComponentConfig::m_inputChannels, "Input Channels", "The entities that provide input data for texture channels")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::ValuesOnly)
                    ->DataElement(AZ::Edit::UIHandlers::CheckBox, &PassComponentConfig::m_isOutputPass, "The Output Pass", "If selected, this pass becomes the output of the RenderJoy pipeline.")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::ValuesOnly)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &PassComponentConfig::m_renderTargetWidth, "Render Target Width", "Not applicable to the output pass because the output pass uses the viewport size.")
                        ->Attribute(AZ::Edit::Attributes::Min, PassComponentConfig::MinRenderTargetSize)
                        ->Attribute(AZ::Edit::Attributes::Max, PassComponentConfig::MaxRenderTargetSize)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::ValuesOnly)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &PassComponentConfig::m_renderTargetHeight, "Render Target Height", "Not applicable to the output pass because the output pass uses the viewport size.")
                        ->Attribute(AZ::Edit::Attributes::Min, PassComponentConfig::MinRenderTargetSize)
                        ->Attribute(AZ::Edit::Attributes::Max, PassComponentConfig::MaxRenderTargetSize)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::ValuesOnly)
                    ;
            }
        }

    }

    void RenderJoyPassEditorComponent::Activate()
    {
        RenderJoyPassRequestBus::Handler::BusConnect(GetEntityId());
    }

    void RenderJoyPassEditorComponent::Deactivate()
    {
        RenderJoyPassRequestBus::Handler::BusDisconnect();
    }

    void RenderJoyPassEditorComponent::OnConfigChanged()
    {
        AZ_TracePrintf(LogName, "Configuration changed\n");
    }


    //////////////////////////////////////////////////////////////////////////////////
    //  AZ::Data::AssetBus overrides
    void RenderJoyPassEditorComponent::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
    }

    void RenderJoyPassEditorComponent::OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
    }

    void RenderJoyPassEditorComponent::OnAssetError(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
    }
    //////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////
    // RenderJoyPassRequestBus interface
    bool RenderJoyPassEditorComponent::IsOutputPass()
    {
        return m_config.m_isOutputPass;
    }

    AZStd::vector<AZ::EntityId> RenderJoyPassEditorComponent::GetEntitiesOnInputChannels()
    {
        AZStd::vector<AZ::EntityId> entities;
        entities.reserve(m_config.m_inputChannels.size());
        for (const auto& entityId : m_config.m_inputChannels)
        {
            entities.push_back(entityId);
        }
        return entities;
    }

    uint32_t RenderJoyPassEditorComponent::GetRenderTargetWidth()
    {
        return m_config.m_renderTargetWidth;
    }

    uint32_t RenderJoyPassEditorComponent::GetRenderTargetHeight()
    {
        return m_config.m_renderTargetHeight;
    }
    //////////////////////////////////////////////////////////////////////////

} // namespace RenderJoy
