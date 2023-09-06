/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Atom/RPI.Public/Pass/PassFilter.h>
#include <Atom/RPI.Public/RenderPipeline.h>

#include <RenderJoy/RenderJoyBus.h>
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
        // Build the list of the entities we will render.
        // Time to get a list of the pass requests.
        auto renderJoySystem = RenderJoyInterface::Get();
        auto entities = renderJoySystem->GetPipelineEntities();
        for (const auto& entityId : entities)
        {
            PipelineEntity pipelineEntity;
            pipelineEntity.m_entityId = entityId;
            m_entities.emplace(entityId, AZStd::move(pipelineEntity));
        }

    }

    void RenderJoyFeatureProcessor::Deactivate()
    {
        m_entities.clear();
    }

    void RenderJoyFeatureProcessor::Simulate([[maybe_unused]] const FeatureProcessor::SimulatePacket& packet)
    {
        
    }


    // TODO: Add to engine at C:\GIT\o3de\Gems\Atom\RPI\Code\Source\RPI.Public\RPIUtils.cpp
    static void MyAddPassRequestToRenderPipeline(
        AZ::RPI::RenderPipeline* renderPipeline,
        const AZ::RPI::PassRequest* passRequest,
        const char* referencePass,
        bool beforeReferencePass)
    {
        // Make sure the pass doesn't exist
        AZ::RPI::PassFilter passFilter = AZ::RPI::PassFilter::CreateWithPassName(passRequest->m_passName, renderPipeline);
        AZ::RPI::Pass* existingPass = AZ::RPI::PassSystemInterface::Get()->FindFirstPass(passFilter);
        AZ_Assert(existingPass == nullptr, "Pass with name %s already exists!", passRequest->m_passName.GetCStr());

        // Create the pass
        AZ::RPI::Ptr<AZ::RPI::Pass> newPass = AZ::RPI::PassSystemInterface::Get()->CreatePassFromRequest(passRequest);
        if (!newPass)
        {
            AZ_Error(LogName, false, "Failed to create the pass from pass request [%s].", passRequest->m_passName.GetCStr());
            return;
        }

        // Add the pass to render pipeline
        bool success;
        if (beforeReferencePass)
        {
            success = renderPipeline->AddPassBefore(newPass, AZ::Name(referencePass));
        }
        else
        {
            success = renderPipeline->AddPassAfter(newPass, AZ::Name(referencePass));
        }
        // only create pass resources if it was success
        if (!success)
        {
            AZ_Error(
                "RPIUtils",
                false,
                "Failed to add pass [%s] to render pipeline [%s].",
                newPass->GetName().GetCStr(),
                renderPipeline->GetId().GetCStr());
        }
    }

    void RenderJoyFeatureProcessor::AddRenderPasses(AZ::RPI::RenderPipeline* renderPipeline)
    {
        auto renderJoySystem = RenderJoyInterface::Get();
        for (auto& itor : m_entities)
        {
            auto entityId = itor.first;
            auto passRequest = renderJoySystem->GetPassRequest(entityId);
            MyAddPassRequestToRenderPipeline(renderPipeline, passRequest.get(), "AuxGeomPass", true);
        }
    }
}
