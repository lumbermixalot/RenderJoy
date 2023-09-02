/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "RenderBoySystemComponent.h"

#include <RenderBoy/RenderBoyTypeIds.h>

#include <AzCore/Serialization/SerializeContext.h>

#include <Atom/RPI.Public/FeatureProcessorFactory.h>

#include <Render/RenderBoyFeatureProcessor.h>

namespace RenderBoy
{
    AZ_COMPONENT_IMPL(RenderBoySystemComponent, "RenderBoySystemComponent",
        RenderBoySystemComponentTypeId);

    void RenderBoySystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderBoySystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }

        RenderBoyFeatureProcessor::Reflect(context);
    }

    void RenderBoySystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("RenderBoySystemService"));
    }

    void RenderBoySystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("RenderBoySystemService"));
    }

    void RenderBoySystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("RPISystem"));
    }

    void RenderBoySystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    RenderBoySystemComponent::RenderBoySystemComponent()
    {
        if (RenderBoyInterface::Get() == nullptr)
        {
            RenderBoyInterface::Register(this);
        }
    }

    RenderBoySystemComponent::~RenderBoySystemComponent()
    {
        if (RenderBoyInterface::Get() == this)
        {
            RenderBoyInterface::Unregister(this);
        }
    }

    void RenderBoySystemComponent::Init()
    {
    }

    void RenderBoySystemComponent::Activate()
    {
        RenderBoyRequestBus::Handler::BusConnect();

        AZ::RPI::FeatureProcessorFactory::Get()->RegisterFeatureProcessor<RenderBoyFeatureProcessor>();
    }

    void RenderBoySystemComponent::Deactivate()
    {
        AZ::RPI::FeatureProcessorFactory::Get()->UnregisterFeatureProcessor<RenderBoyFeatureProcessor>();

        RenderBoyRequestBus::Handler::BusDisconnect();
    }

} // namespace RenderBoy
