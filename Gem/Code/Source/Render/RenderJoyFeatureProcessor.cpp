/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Component/TickBus.h>

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
                ->Class<RenderJoyFeatureProcessor, AZ::RPI::FeatureProcessor>()
                ;
        }
    }

    void RenderJoyFeatureProcessor::Activate()
    {
        AZ_Printf(LogName, "%s\n", __FUNCTION__);
        // Activation is broadcasted after all passes have been added to the main render pipeline.
    }

    void RenderJoyFeatureProcessor::Deactivate()
    {
        AZ_Printf(LogName, "%s\n", __FUNCTION__);
        RenderJoyNotificationBus::Broadcast(&RenderJoyNotifications::OnFeatureProcessorDeactivated);
    }

    void RenderJoyFeatureProcessor::Simulate([[maybe_unused]] const FeatureProcessor::SimulatePacket& packet)
    {
        
    }


    // TODO: Add to engine at C:\GIT\o3de\Gems\Atom\RPI\Code\Source\RPI.Public\RPIUtils.cpp
    void RenderJoyFeatureProcessor::MyAddPassRequestToRenderPipeline(
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
                LogName,
                false,
                "Failed to add pass [%s] to render pipeline [%s].",
                newPass->GetName().GetCStr(),
                renderPipeline->GetId().GetCStr());
        }
    }

    void RenderJoyFeatureProcessor::AddRenderPasses(AZ::RPI::RenderPipeline* renderPipeline)
    {
        auto renderJoySystem = RenderJoyInterface::Get();
        auto parentPassEntities = renderJoySystem->GetParentPassEntities();
        m_parentPassCount = static_cast<uint32_t>(parentPassEntities.size());
        m_activedParentPassCount = 0;
        for (const auto& entityId  : parentPassEntities)
        {
            auto passRequest = renderJoySystem->GetPassRequest(entityId);
            MyAddPassRequestToRenderPipeline(renderPipeline, passRequest.get(), "AuxGeomPass", true);
        }
    }

    // RenderJoyFeatureProcessorInterface overrides
    void RenderJoyFeatureProcessor::OnBillboardPassReady(const AZ::Name& billboardPassName)
    {
        AZ_Printf(LogName, "%s pass name=%s.\n", __FUNCTION__, billboardPassName.GetCStr());
        m_activedParentPassCount++;
        if (m_activedParentPassCount == m_parentPassCount)
        {
            // Queue it for next frame. to avoid spurious crashes.
            auto notifyAssetchangedFn = []()
                {
                    RenderJoyNotificationBus::Broadcast(&RenderJoyNotifications::OnFeatureProcessorActivated);
                };
            AZ::TickBus::QueueFunction(AZStd::move(notifyAssetchangedFn));
        }
    }
}
