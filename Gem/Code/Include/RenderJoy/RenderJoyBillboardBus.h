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

namespace RenderJoy
{
    class RenderJoyBillboardRequests
        : public AZ::EBusTraits
    {
    public:
        // AZ_RTTI required on this EBUS, this allows us to iterate through the handlers of this EBUS and deduce their type.
        AZ_RTTI(RenderJoyBillboardRequests, "{CAFE72AA-1ACE-4AE9-9CD4-942F7D55C185}");

        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId; // An Entity with a RenderJoyPassEditorComponent.
        //////////////////////////////////////////////////////////////////////////

        virtual void SetFlatscreenMode(float posX, float posY, float width, float height) = 0;
        virtual void SetBillboardMode(bool alwaysFaceCamera) = 0;

    };
    using RenderJoyBillboardBus = AZ::EBus<RenderJoyBillboardRequests>;

} // namespace RenderJoy
