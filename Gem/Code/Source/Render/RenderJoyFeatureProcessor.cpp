/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "RenderJoyFeatureProcessor.h"

namespace RenderJoy
{
    void RenderJoyFeatureProcessor::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext
                ->Class<RenderJoyFeatureProcessor, FeatureProcessor>()
                ;
        }
    }

    void RenderJoyFeatureProcessor::Activate()
    {

    }

    void RenderJoyFeatureProcessor::Deactivate()
    {
        
    }

    void RenderJoyFeatureProcessor::Simulate([[maybe_unused]] const FeatureProcessor::SimulatePacket& packet)
    {
        
    }    
}
