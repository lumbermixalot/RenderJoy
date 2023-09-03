/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <RenderJoy/RenderJoyTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace RenderJoy
{
    class RenderJoyRequests
    {
    public:
        AZ_RTTI(RenderJoyRequests, RenderJoyRequestsTypeId);
        virtual ~RenderJoyRequests() = default;
        // Put your public methods here
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
