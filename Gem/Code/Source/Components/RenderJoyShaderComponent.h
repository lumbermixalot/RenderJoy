/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Components/RenderJoyShaderComponentController.h>
#include <AzFramework/Components/ComponentAdapter.h>

namespace RenderJoy
{
    inline constexpr AZ::TypeId RenderJoyShaderComponentTypeId { "{330D4851-8936-47A0-83A7-17543F92E238}" };

    class RenderJoyShaderComponent final
        : public AzFramework::Components::ComponentAdapter<RenderJoyShaderComponentController, RenderJoyShaderComponentConfig>
    {
    public:
        using BaseClass = AzFramework::Components::ComponentAdapter<RenderJoyShaderComponentController, RenderJoyShaderComponentConfig>;
        AZ_COMPONENT(RenderJoyShaderComponent, RenderJoyShaderComponentTypeId, BaseClass);

        RenderJoyShaderComponent() = default;
        RenderJoyShaderComponent(const RenderJoyShaderComponentConfig& config);

        static void Reflect(AZ::ReflectContext* context);
    };
}
