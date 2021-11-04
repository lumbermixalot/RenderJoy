/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>

namespace RenderJoy
{
    class RenderJoyKeyboardNotifications
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        //////////////////////////////////////////////////////////////////////////

        struct KeyCodeInfo
        {
            uint32_t m_virtualScanCode; // bitwise AND with 0xff, and you'll get the JavaScript virtual key code used by ShaderToy.
            uint32_t m_nativeScanCode;
            int      m_qtKeyCode;
        };

        virtual void OnKeyDown(const KeyCodeInfo& keyCodeInfo) = 0;
        virtual void OnKeyUp(const KeyCodeInfo& keyCodeInfo) = 0;

    };

    using RenderJoyKeyboardNotificationBus = AZ::EBus<RenderJoyKeyboardNotifications>;

} // namespace RenderJoy
