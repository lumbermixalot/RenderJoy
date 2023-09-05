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
        virtual PassTemplateOutcome CreateRenderJoyPassTemplate(AZ::EntityId passBusEntity) const = 0;

        // If the feature processor doesn't exist, it will create one.
        // Adds a cosmetic billboard to the scene with a texture highlighting that the
        // RenderJoy pipeline is invalid.
        // Returns true if it was successful.
        // @param pipelineEntityId The entity that owns a RenderJoy pipeline (many RenderJoy pipelines can co-exist).
        // @param passBusEntity This is the entity that implements the RenderJoyPassBus interface, and it is assumed to be the output pass.
        //                      This entity belongs to @pipelineEntityId.
        virtual bool AddInvalidRenderJoyPipeline(AZ::EntityId pipelineEntityId, AZ::EntityId passBusEntity) = 0;

        // Adds one more RenderJoy render pipeline to the scene. It is assumed that @passTemplate is valid and that it was created
        // by calling CreateRenderJoyPassTemplate().
        // @param pipelineEntityId The entity that owns a RenderJoy pipeline (many RenderJoy pipelines can co-exist).
        // @param passBusEntity This is the entity that implements the RenderJoyPassBus interface, and it is assumed to be the output pass.
        //                      This entity belongs to @pipelineEntityId.
        virtual bool AddRenderJoyPipeline(AZ::EntityId pipelineEntityId, AZ::EntityId passBusEntity, const AZ::RPI::PassTemplate& passTemplate) = 0;

        // Removes a particular pipeline, if no pipelines are left it will destroy the feature processor.
        virtual void RemoveRenderJoyPipeline(AZ::EntityId pipelineEntityId) = 0;
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
