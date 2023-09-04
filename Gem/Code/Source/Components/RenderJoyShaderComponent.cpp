/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Components/RenderJoyShaderComponent.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace RenderJoy
{
    RenderJoyShaderComponent::RenderJoyShaderComponent(const RenderJoyShaderComponentConfig& config)
        : BaseClass(config)
    {
    }

    void RenderJoyShaderComponent::Reflect(AZ::ReflectContext* context)
    {
        BaseClass::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyShaderComponent, BaseClass>()
                ->Version(0)
                ;
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->ConstantProperty("RenderJoyShaderComponentTypeId", BehaviorConstant(AZ::Uuid(RenderJoyShaderComponentTypeId)))
                ->Attribute(AZ::Script::Attributes::Module, "render")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common);
        }
    }
}
