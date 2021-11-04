/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Math/Vector2.h>

namespace RenderJoy
{
    class RenderJoySettingsRequests
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        //////////////////////////////////////////////////////////////////////////

        virtual AZ::Vector2 GetRenderTargetSize() const = 0;
    };
    using RenderJoySettingsRequestBus = AZ::EBus<RenderJoySettingsRequests>;

    class RenderJoySettingsNotifications
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        //////////////////////////////////////////////////////////////////////////

        virtual void OnRenderTargetSizeChanged(const AZ::Vector2& newSize) = 0;
    };
    using RenderJoySettingsNotificationBus = AZ::EBus<RenderJoySettingsNotifications>;

} // namespace RenderJoy
