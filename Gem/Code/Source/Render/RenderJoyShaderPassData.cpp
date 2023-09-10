/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/
#include <AzCore/Serialization/SerializeContext.h>

#include "RenderJoyShaderPassData.h"

namespace RenderJoy
{
    void RenderJoyShaderPassData::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyShaderPassData, AZ::RPI::RenderPassData>()
                ->Version(0)
                ->Field("ShaderAsset", &RenderJoyShaderPassData::m_shaderAsset)
                ->Field("StencilRef", &RenderJoyShaderPassData::m_stencilRef);
        }
    }
} // namespace RenderJoy