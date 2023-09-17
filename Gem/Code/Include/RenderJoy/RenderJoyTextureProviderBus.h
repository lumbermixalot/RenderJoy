/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Component/EntityId.h>

#include <Atom/RPI.Reflect/Image/Image.h>

namespace RenderJoy
{
    class RenderJoyTextureProvider
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        typedef AZ::EntityId BusIdType; // The EntityId that provides the texture
        //////////////////////////////////////////////////////////////////////////

        virtual AZ::Data::Instance<AZ::RPI::Image> GetImage() const = 0;
    };
    using RenderJoyTextureProviderBus = AZ::EBus<RenderJoyTextureProvider>;

    class RenderJoyTextureProviderNotification
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        typedef AZ::EntityId BusIdType; // The EntityId that provides the texture
        //////////////////////////////////////////////////////////////////////////

        virtual void OnImageChanged(AZ::Data::Instance<AZ::RPI::Image>) = 0;

    };
    using RenderJoyTextureProviderNotificationBus = AZ::EBus<RenderJoyTextureProviderNotification>;

    namespace Utils
    {
        // Returns true is the entity implements the RenderJoyTextureProviderBus.
        bool IsRenderJoyTextureProvider(AZ::EntityId entityId);
    }
} // namespace RenderJoy
