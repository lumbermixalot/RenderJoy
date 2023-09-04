/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <RenderJoy/RenderJoyTypeIds.h>
#include <RenderJoyModuleInterface.h>
#include "RenderJoySystemComponent.h"

#include <AzCore/RTTI/RTTI.h>

#include "Components/RenderJoyBillboardComponent.h"
#include "Components/RenderJoyShaderComponent.h"

namespace RenderJoy
{
    class RenderJoyModule
        : public RenderJoyModuleInterface
    {
    public:
        AZ_RTTI(RenderJoyModule, RenderJoyModuleTypeId, RenderJoyModuleInterface);
        AZ_CLASS_ALLOCATOR(RenderJoyModule, AZ::SystemAllocator);

        RenderJoyModule()
        {
            m_descriptors.insert(m_descriptors.end(),
                {
                    RenderJoySystemComponent::CreateDescriptor(),
                    RenderJoyBillboardComponent::CreateDescriptor(),
                    RenderJoyShaderComponent::CreateDescriptor(),
                });
        }

        AZ::ComponentTypeList GetRequiredSystemComponents() const
        {
            return AZ::ComponentTypeList{ azrtti_typeid<RenderJoySystemComponent>() };
        }
    };
}// namespace RenderJoy

AZ_DECLARE_MODULE_CLASS(Gem_RenderJoy, RenderJoy::RenderJoyModule)
