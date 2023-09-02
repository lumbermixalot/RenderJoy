/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <RenderBoy/RenderBoyTypeIds.h>
#include <RenderBoyModuleInterface.h>
#include "RenderBoySystemComponent.h"

#include <AzCore/RTTI/RTTI.h>

#include <Components/RenderBoyComponent.h>

namespace RenderBoy
{
    class RenderBoyModule
        : public RenderBoyModuleInterface
    {
    public:
        AZ_RTTI(RenderBoyModule, RenderBoyModuleTypeId, RenderBoyModuleInterface);
        AZ_CLASS_ALLOCATOR(RenderBoyModule, AZ::SystemAllocator);

        RenderBoyModule()
        {
            m_descriptors.insert(m_descriptors.end(),
                {
                    RenderBoySystemComponent::CreateDescriptor(),
                    RenderBoyComponent::CreateDescriptor(),
                });
        }

        AZ::ComponentTypeList GetRequiredSystemComponents() const
        {
            return AZ::ComponentTypeList{ azrtti_typeid<RenderBoySystemComponent>() };
        }
    };
}// namespace RenderBoy

AZ_DECLARE_MODULE_CLASS(Gem_RenderBoy, RenderBoy::RenderBoyModule)
