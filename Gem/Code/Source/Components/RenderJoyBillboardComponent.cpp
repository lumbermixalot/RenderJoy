/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <Components/RenderJoyBillboardComponent.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace RenderJoy
{
    RenderJoyBillboardComponent::RenderJoyBillboardComponent(const RenderJoyBillboardComponentConfig& config)
        : BaseClass(config)
    {
    }

    void RenderJoyBillboardComponent::Reflect(AZ::ReflectContext* context)
    {
        BaseClass::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyBillboardComponent, BaseClass>()
                ->Version(0)
                ;
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->ConstantProperty("RenderJoyBillboardComponentTypeId", BehaviorConstant(AZ::Uuid(RenderJoyBillboardComponentTypeId)))
                ->Attribute(AZ::Script::Attributes::Module, "render")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common);
        }
    }
}
