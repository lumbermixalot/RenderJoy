/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <RenderJoy/RenderJoyFeatureProcessorInterface.h>

namespace RenderJoy
{
    class RenderJoyFeatureProcessor final
        : public RenderJoyFeatureProcessorInterface
    {
    public:
        AZ_RTTI(RenderJoyFeatureProcessor, "{36E700E8-A60E-4EFF-BA17-8E6F7352698C}", RenderJoyFeatureProcessorInterface);
        AZ_CLASS_ALLOCATOR(RenderJoyFeatureProcessor, AZ::SystemAllocator)

        static void Reflect(AZ::ReflectContext* context);

        RenderJoyFeatureProcessor() = default;
        virtual ~RenderJoyFeatureProcessor() = default;

        // FeatureProcessor overrides
        void Activate() override;
        void Deactivate() override;
        void Simulate(const FeatureProcessor::SimulatePacket& packet) override;

    };
}
