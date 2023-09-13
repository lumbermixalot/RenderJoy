/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <Components/RenderJoyTextureComponent.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace RenderJoy
{
    RenderJoyTextureComponent::RenderJoyTextureComponent(const RenderJoyTextureComponentConfig& config)
        : BaseClass(config)
    {
    }

    void RenderJoyTextureComponent::Reflect(AZ::ReflectContext* context)
    {
        BaseClass::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyTextureComponent, BaseClass>()
                ->Version(0)
                ;
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->ConstantProperty("RenderJoyTextureComponentTypeId", BehaviorConstant(AZ::Uuid(RenderJoyTextureComponentTypeId)))
                ->Attribute(AZ::Script::Attributes::Module, "render")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common);
        }
    }
}
