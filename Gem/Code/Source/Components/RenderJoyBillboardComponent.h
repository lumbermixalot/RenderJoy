/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <Components/RenderJoyBillboardComponentController.h>
#include <AzFramework/Components/ComponentAdapter.h>

namespace RenderJoy
{
    inline constexpr AZ::TypeId RenderJoyBillboardComponentTypeId { "{44F868F4-698C-4183-ABBF-A8BC5E6FA040}" };

    class RenderJoyBillboardComponent final
        : public AzFramework::Components::ComponentAdapter<RenderJoyBillboardComponentController, RenderJoyBillboardComponentConfig>
    {
    public:
        using BaseClass = AzFramework::Components::ComponentAdapter<RenderJoyBillboardComponentController, RenderJoyBillboardComponentConfig>;
        AZ_COMPONENT(RenderJoyBillboardComponent, RenderJoyBillboardComponentTypeId, BaseClass);

        RenderJoyBillboardComponent() = default;
        RenderJoyBillboardComponent(const RenderJoyBillboardComponentConfig& config);

        static void Reflect(AZ::ReflectContext* context);
    };
}
