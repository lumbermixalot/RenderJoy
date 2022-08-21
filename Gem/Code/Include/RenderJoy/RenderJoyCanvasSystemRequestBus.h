/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>

namespace AtomToolsFramework
{
    struct GraphViewConfig;
}

namespace RenderJoy
{
    class RenderJoyCanvasSystemRequests
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        //////////////////////////////////////////////////////////////////////////

        virtual void GetRenderJoyGraphViewConfig(AtomToolsFramework::GraphViewConfig& graphViewConfig) const = 0;
    };
    using RenderJoyCanvasSystemRequestBus = AZ::EBus<RenderJoyCanvasSystemRequests>;

    //class RenderJoySettingsNotifications
    //    : public AZ::EBusTraits
    //{
    //public:
    //    //////////////////////////////////////////////////////////////////////////
    //    // EBusTraits overrides
    //    static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
    //    //////////////////////////////////////////////////////////////////////////
    //
    //    virtual void OnRenderTargetSizeChanged(const AZ::Vector2& newSize) = 0;
    //};
    //using RenderJoySettingsNotificationBus = AZ::EBus<RenderJoySettingsNotifications>;

} // namespace RenderJoy
