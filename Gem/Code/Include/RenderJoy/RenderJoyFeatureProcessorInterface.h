/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzCore/base.h>
#include <Atom/RPI.Public/FeatureProcessor.h>

namespace RenderJoy
{
    // RenderJoyFeatureProcessorInterface provides an interface to the feature processor for code outside of Atom
    class RenderJoyFeatureProcessorInterface
        : public AZ::RPI::FeatureProcessor
    {
    public:
        AZ_RTTI(RenderJoyFeatureProcessorInterface, "{96DDAA2F-C947-42BA-A664-453C440D7DAE}", AZ::RPI::FeatureProcessor);

    };
}
