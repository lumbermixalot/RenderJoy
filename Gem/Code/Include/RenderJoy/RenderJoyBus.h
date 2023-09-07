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

#include <Atom/RPI.Reflect/Pass/PassRequest.h>

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

        // Adds one more RenderJoy render pipeline to the scene.
        // Returns true if the pipeline was created successfully.
        // @param pipelineEntityId The entity that owns a RenderJoy pipeline (many RenderJoy pipelines can co-exist).
        // @param passBusEntity This is the entity that implements the RenderJoyPassBus interface, and it is assumed to be the output pass.
        //                      This entity belongs to @pipelineEntityId.
        // REMARK: If the RenderJoy pipeline is not properly configured this function may still succeed because it will attempt
        //         to create a dummy pipeline that shows a billboard with a texture saying that the pipeline is invalid.
        virtual bool AddRenderJoyPipeline(AZ::EntityId pipelineEntityId, AZ::EntityId passBusEntity) = 0;

        // Removes a particular pipeline, if no pipelines are left it will destroy the feature processor.
        // Returns false if @pipelineEntityId was not previously added.
        virtual bool RemoveRenderJoyPipeline(AZ::EntityId pipelineEntityId) = 0;

        // Returns a list of pipeline entities.
        virtual AZStd::vector<AZ::EntityId> GetPipelineEntities() const = 0;
        virtual AZStd::shared_ptr<AZ::RPI::PassRequest> GetPassRequest(AZ::EntityId pipelineEntityId) const = 0;
        virtual AZ::Name GetBillboardPassName(AZ::EntityId pipelineEntityId) const = 0;
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


    class RenderJoyNotifications
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    
        virtual void OnFeatureProcessorActivated() = 0;
        virtual void OnFeatureProcessorDeactivated() = 0;
    };
    
    using RenderJoyNotificationBus = AZ::EBus<RenderJoyNotifications>;

} // namespace RenderJoy
