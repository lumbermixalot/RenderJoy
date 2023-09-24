/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
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
        struct RenderJoyShaderPassData
            : public AZ::RPI::RenderPassData
        {
            AZ_RTTI(RenderJoyShaderPassData, "{B29491DA-E55D-462B-BB2C-9AA0F3A01366}", AZ::RPI::RenderPassData);
            AZ_CLASS_ALLOCATOR(RenderJoyShaderPassData, AZ::SystemAllocator);

            RenderJoyShaderPassData() = default;
            virtual ~RenderJoyShaderPassData() = default;

            static void Reflect(AZ::ReflectContext* context);

            //! Reference to the shader asset used by the fullscreen triangle pass
            AZ::RPI::AssetReference m_shaderAsset;

            //! Stencil reference value to use for the draw call
            uint32_t m_stencilRef = 0;

            uint32_t m_renderTargetWidth = 0;
            uint32_t m_renderTargetHeight = 0;

        };
} // namespace RenderJoy

