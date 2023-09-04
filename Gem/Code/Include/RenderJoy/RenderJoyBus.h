/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

#include <Atom/RPI.Reflect/Pass/PassTemplate.h>

#include <RenderJoy/RenderJoyTypeIds.h>


namespace RenderJoy
{
    using PassTemplateOutcome = AZ::Outcome<AZ::RPI::PassTemplate, AZStd::string>;

    class RenderJoyRequests
    {
    public:
        AZ_RTTI(RenderJoyRequests, RenderJoyRequestsTypeId);
        virtual ~RenderJoyRequests() = default;
        
        // Put your public methods here
        
        // Creates the pass template (with an embedded pass request) that is valid and can be used
        // to instantiate the whole RenderJoy parent pass.
        // The PassTemplate is created from the Entities that own RenderJoy-related components.
        // @param passBusEntity An entity that implements the RenderJoyPassBus interface.
        virtual PassTemplateOutcome CreateRenderJoyPassTemplate(AZ::EntityId passBusEntity) = 0;

        // The RenderJoy Feature processsor is never created by default, even if you enable this Gem.
        // Only when one of the Output-type of components is instantiated, 
        virtual bool CreateFeatureProcessor(const AZ::RPI::PassTemplate& passTemplate) = 0;

        // Destroy the RenderJoy Feature processor if it exists. Otherwise, does nothing.
        virtual void DestroyFeatureProcessor() = 0;
    };

    class RenderJoyBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using RenderJoyRequestBus = AZ::EBus<RenderJoyRequests, RenderJoyBusTraits>;
    using RenderJoyInterface = AZ::Interface<RenderJoyRequests>;

} // namespace RenderJoy
