/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <Components/RenderJoyTextureComponentController.h>
#include <AzFramework/Components/ComponentAdapter.h>

namespace RenderJoy
{
    inline constexpr AZ::TypeId RenderJoyTextureComponentTypeId { "{8E453275-B2F7-4F2F-B31D-D0234E5D8416}" };

    class RenderJoyTextureComponent final
        : public AzFramework::Components::ComponentAdapter<RenderJoyTextureComponentController, RenderJoyTextureComponentConfig>
    {
    public:
        using BaseClass = AzFramework::Components::ComponentAdapter<RenderJoyTextureComponentController, RenderJoyTextureComponentConfig>;
        AZ_COMPONENT(RenderJoyTextureComponent, RenderJoyTextureComponentTypeId, BaseClass);

        RenderJoyTextureComponent() = default;
        RenderJoyTextureComponent(const RenderJoyTextureComponentConfig& config);

        static void Reflect(AZ::ReflectContext* context);
    };
}
