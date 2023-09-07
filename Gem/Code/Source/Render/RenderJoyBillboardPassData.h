/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <Atom/RPI.Reflect/Asset/AssetReference.h>
#include <Atom/RPI.Reflect/Pass/RenderPassData.h>

namespace RenderJoy
{
        //! Custom data for the FullscreenTrianglePass. Should be specified in the PassRequest.
        struct RenderJoyBillboardPassData
            : public AZ::RPI::RenderPassData
        {
            AZ_RTTI(RenderJoyBillboardPassData, "{C32CB191-BA25-4BF0-9582-4D59CB936CCF}", AZ::RPI::RenderPassData);
            AZ_CLASS_ALLOCATOR(RenderJoyBillboardPassData, AZ::SystemAllocator);

            RenderJoyBillboardPassData() = default;
            virtual ~RenderJoyBillboardPassData() = default;

            static void Reflect(AZ::ReflectContext* context);

            //! Reference to the shader asset used by the fullscreen triangle pass
            AZ::RPI::AssetReference m_shaderAsset;

            //! Stencil reference value to use for the draw call
            uint32_t m_stencilRef = 0;

        };
} // namespace RenderJoy

