/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>

#include <RenderJoy/RenderJoyBus.h>
#include <RenderJoy/RenderJoyFeatureProcessorInterface.h>

#include "RenderJoyTemplatesFactory.h"

namespace AZ
{
    namespace RPI
    {
        class Scene;
    }
}

namespace RenderJoy
{
    class RenderJoySystemComponent
        : public AZ::Component
        , public RenderJoyRequestBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(RenderJoySystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        RenderJoySystemComponent();
        ~RenderJoySystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // RenderJoyRequestBus interface implementation START
        PassTemplateOutcome CreateRenderJoyPassTemplate(AZ::EntityId passBusEntity) const override;
        bool AddInvalidRenderJoyPipeline(AZ::EntityId pipelineEntityId, AZ::EntityId passBusEntity) override;
        bool AddRenderJoyPipeline(AZ::EntityId pipelineEntityId, AZ::EntityId passBusEntity, const AZ::RPI::PassTemplate& passTemplate) override;
        void RemoveRenderJoyPipeline(AZ::EntityId pipelineEntityId) override;
        // RenderJoyRequestBus interface implementation END
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

    private:
        static constexpr char LogName[] = "RenderJoySystemComponent";

        void DestroyFeatureProcessor();
        void CreateFeatureProcessor();

        // It is assumed that all RenderJoy related passes go to the same scene.
        AZ::RPI::Scene* m_scenePtr = nullptr;
        RenderJoyFeatureProcessorInterface* m_featureProcessor = nullptr;

        RenderJoyTemplatesFactory m_templatesFactory;

        // The key is the EntityId that owns a billboard, the value is the name of the pass template
        AZStd::unordered_map<AZ::EntityId, AZ::Name> m_passInstances;


    };

} // namespace RenderJoy
