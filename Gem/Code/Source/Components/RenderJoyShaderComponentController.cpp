/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <Components/RenderJoyShaderComponentController.h>

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
        provided.push_back(AZ_CRC_CE("RenderJoyService"));
    }

    void RenderJoyShaderComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("RenderJoyService"));
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
        m_entityId = entityId;

    }

    void RenderJoyShaderComponentController::Deactivate()
    {

    }

    void RenderJoyShaderComponentController::SetConfiguration(const RenderJoyShaderComponentConfig& config)
    {
        m_configuration = config;
    }

    const RenderJoyShaderComponentConfig& RenderJoyShaderComponentController::GetConfiguration() const
    {
        return m_configuration;
    }

    void RenderJoyShaderComponentController::OnConfigurationChanged()
    {
        AZ_Printf(LogName, "%s\n", __FUNCTION__);
    }

    /////////////////////////////////////////////////////////////////
    /// RenderJoyPassRequestBus::Handler overrides START
    AZStd::vector<AZ::EntityId> RenderJoyShaderComponentController::GetEntitiesOnInputChannels()
    {
        AZStd::vector<AZ::EntityId> retList;
        retList.reserve(m_configuration.m_inputChannels.size());

        for (const auto& entityId : m_configuration.m_inputChannels)
        {
            retList.push_back(entityId);
        }

        return retList;
    }

    AZ::Data::Asset<AZ::RPI::ShaderAsset> RenderJoyShaderComponentController::GetShaderAsset()
    {
        return m_configuration.m_shaderAsset;
    }

    uint32_t RenderJoyShaderComponentController::GetRenderTargetWidth()
    {
        return m_configuration.m_renderTargetWidth;
    }

    uint32_t RenderJoyShaderComponentController::GetRenderTargetHeight()
    {
        return m_configuration.m_renderTargetHeight;
    }
    /// RenderJoyPassRequestBus::Handler overrides END
    /////////////////////////////////////////////////////////////////

}
