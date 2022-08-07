/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>

namespace RenderJoy
{
    class RenderJoyDocumentNotifications : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        typedef AZ::Crc32 BusIdType;

        // This notification is sent whenever graph compilation has completed.
        virtual void OnCompileGraphCompleted([[maybe_unused]] const AZ::Uuid& documentId){};
    };

    using RenderJoyDocumentNotificationBus = AZ::EBus<RenderJoyDocumentNotifications>;
} // namespace RenderJoy
