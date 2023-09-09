/*
* Copyright (c) Contributors to the Open 3D Engine Project.
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <AzCore/Serialization/SerializeContext.h>


#include "RenderJoyBillboardPassData.h"

namespace RenderJoy
{
    void RenderJoyBillboardPassData::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RenderJoyBillboardPassData, AZ::RPI::RenderPassData>()
                ->Version(0)
                ->Field("InputTextureIsAttachment", &RenderJoyBillboardPassData::m_inputTextureIsAttachment)
                ->Field("StencilRef", &RenderJoyBillboardPassData::m_stencilRef)
                ;
        }
    }
} //namespace RenderJoy