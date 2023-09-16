/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Interface/Interface.h>
#include <AzCore/Math/Vector2.h>

namespace RenderJoy
{
    //! This is the AZ::Interface<> declaration for the singleton responsible
    //! for providing per frame data for RenderJoy shaders.
    class IRenderJoySrgDataProvider
    {
    public:
        AZ_RTTI(IRenderJoySrgDataProvider, "{A7C2F151-4A15-4B17-B932-D966BE4BB6C5}");

        static constexpr const char* LogName = "IRenderJoySrgDataProvider";

        virtual ~IRenderJoySrgDataProvider() = default;

        virtual float GetTime() = 0;
        virtual float GetTimeDelta() = 0;
        virtual int GetFramesCount() = 0;
        virtual float GetFramesPerSecond() = 0;
        virtual void GetMouseData(AZ::Vector2& currentPos, AZ::Vector2& clickPos, bool& isLeftButtonDown, bool& isLeftButtonClick) = 0;
        virtual void ResetFrameCounter(int newValue) = 0;
    };

    using RenderJoySrgDataProviderInterface = AZ::Interface<IRenderJoySrgDataProvider>;

} // namespace RenderJoy
