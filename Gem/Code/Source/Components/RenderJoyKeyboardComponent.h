/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <Components/RenderJoyKeyboardComponentController.h>
#include <AzFramework/Components/ComponentAdapter.h>

namespace RenderJoy
{
    inline constexpr AZ::TypeId RenderJoyKeyboardComponentTypeId { "{9EFEFBC8-ED9E-4B4E-8A8D-EC86958ADC5A}" };

    class RenderJoyKeyboardComponent final
        : public AzFramework::Components::ComponentAdapter<RenderJoyKeyboardComponentController, RenderJoyKeyboardComponentConfig>
    {
    public:
        using BaseClass = AzFramework::Components::ComponentAdapter<RenderJoyKeyboardComponentController, RenderJoyKeyboardComponentConfig>;
        AZ_COMPONENT(RenderJoyKeyboardComponent, RenderJoyKeyboardComponentTypeId, BaseClass);

        RenderJoyKeyboardComponent() = default;
        RenderJoyKeyboardComponent(const RenderJoyKeyboardComponentConfig& config);

        static void Reflect(AZ::ReflectContext* context);
    };
}
