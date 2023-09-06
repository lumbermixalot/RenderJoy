/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <RenderJoy/RenderJoyFeatureProcessorInterface.h>

class AZ::RPI::RenderPipeline;
class AZ::RPI::PassRequest;

namespace RenderJoy
{
    // This feature processor is in charge of creating all RenderJoy passes
    // and adding them to to the main pipeline.
    class RenderJoyFeatureProcessor final
        : public RenderJoyFeatureProcessorInterface
    {
    public:
        AZ_RTTI(RenderJoyFeatureProcessor, "{36E700E8-A60E-4EFF-BA17-8E6F7352698C}", RenderJoyFeatureProcessorInterface);
        AZ_CLASS_ALLOCATOR(RenderJoyFeatureProcessor, AZ::SystemAllocator)

        static void Reflect(AZ::ReflectContext* context);

        RenderJoyFeatureProcessor() = default;
        virtual ~RenderJoyFeatureProcessor() = default;


    private:
        RenderJoyFeatureProcessor(const RenderJoyFeatureProcessor&) = delete;

        static constexpr char LogName[] = "RenderJoyFeatureProcessor";

        // TODO: Add to engine at C:\GIT\o3de\Gems\Atom\RPI\Code\Source\RPI.Public\RPIUtils.cpp
        static void MyAddPassRequestToRenderPipeline(
            AZ::RPI::RenderPipeline* renderPipeline,
            const AZ::RPI::PassRequest* passRequest,
            const char* referencePass,
            bool beforeReferencePass);

        // FeatureProcessor overrides
        void Activate() override;
        void Deactivate() override;
        void Simulate(const FeatureProcessor::SimulatePacket& packet) override;
        void AddRenderPasses(AZ::RPI::RenderPipeline* renderPipeline) override;

        struct PipelineEntity
        {
            AZ::EntityId m_entityId;
        };

        AZStd::unordered_map<AZ::EntityId, PipelineEntity> m_entities;
    };
}
