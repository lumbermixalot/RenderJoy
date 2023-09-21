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
#include <Atom/RPI.Reflect/Image/Image.h>

#include <RenderJoy/RenderJoyTypeIds.h>


namespace RenderJoy
{
    //using PassTemplateOutcome = AZ::Outcome<AZ::RPI::PassTemplate, AZStd::string>;

    class RenderJoyRequests
    {
    public:
        AZ_RTTI(RenderJoyRequests, RenderJoyRequestsTypeId);
        virtual ~RenderJoyRequests() = default;
        
        // Put your public methods here

        // Adds one RenderJoy parent pass to the scene.
        // Returns true if the parent pass was created successfully.
        // Replaces an existing parent pass if its entityId was found.
        // @param parentPassEntityId The entity that owns a RenderJoy parent pass (many RenderJoy parent passes can co-exist).
        // @param passBusEntity This is the entity that implements the RenderJoyPassBus interface, and it is assumed to be the output pass.
        //                      This entity belongs to @parentPassEntityId.
        // REMARK: If the RenderJoy parent pass is not properly configured this function may still succeed because it will attempt
        //         to create a dummy parent pass that shows a billboard with a texture saying that the parent pass is invalid.
        virtual bool AddRenderJoyParentPass(AZ::EntityId parentPassEntityId, AZ::EntityId passBusEntity) = 0;

        // Removes a particular parent pass, if no parent passes are left it will destroy the feature processor.
        // Returns false if @parentPassEntityId was not previously added.
        virtual bool RemoveRenderJoyParentPass(AZ::EntityId parentPassEntityId) = 0;

        virtual AZStd::vector<AZ::EntityId> GetParentPassEntities() const = 0;
        virtual AZStd::shared_ptr<AZ::RPI::PassRequest> GetPassRequest(AZ::EntityId parentPassEntityId) const = 0;
        virtual AZ::Name GetBillboardPassName(AZ::EntityId parentPassEntityId) const = 0;
        virtual AZ::Name GetShaderPassName(AZ::EntityId shaderPassEntityId) const = 0;

        // This is the texture we display on a billboard or output pass when a RenderJoy ParentPass is invalid.
        virtual AZ::Data::Instance<AZ::RPI::Image> GetInvalidParentPassTexture() const = 0;
        virtual AZ::Data::Instance<AZ::RPI::Image> GetDefaultInputTexture(uint32_t channelIndex) const = 0;

        virtual AZ::EntityId GetEntityIdFromPassName(const AZ::Name& passName) const = 0;
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
        AZ_RTTI(RenderJoyNotifications, RenderJoyNotificationsTypeId);
        virtual ~RenderJoyNotifications() = default;

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
