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

#include <Atom/RPI.Reflect/Asset/AssetUtils.h>
#include <Atom/RPI.Public/Image/StreamingImage.h>

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
        , public AZ::SystemTickBus::Handler
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
        bool AddRenderJoyParentPass(AZ::EntityId parentPassEntityId, AZ::EntityId passBusEntity) override;
        bool RemoveRenderJoyParentPass(AZ::EntityId parentPassEntityId) override;
        AZStd::vector<AZ::EntityId> GetParentPassEntities() const override;
        AZStd::shared_ptr<AZ::RPI::PassRequest> GetPassRequest(AZ::EntityId parentPassEntityId) const override;
        AZ::Name GetBillboardPassName(AZ::EntityId parentPassEntityId) const override;
        AZ::Data::Instance<AZ::RPI::Image> GetInvalidParentPassTexture() const override;
        // RenderJoyRequestBus interface implementation END
        ////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // SystemTickBus
        void OnSystemTick() override;
        //////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

        bool m_shouldRecreateFeatureProcessor = true;

    private:
        static constexpr char LogName[] = "RenderJoySystemComponent";
        static constexpr char InvalidPipelineTexturePath[] =  "textures/renderjoy/invalidpipeline.png.streamingimage"; //"Textures/RenderJoy/InvalidPipeline.png"; 

        void DestroyFeatureProcessor();
        void CreateFeatureProcessor();

        // It is assumed that all RenderJoy related passes go to the same scene.
        AZ::RPI::Scene* m_scenePtr = nullptr;
        RenderJoyFeatureProcessorInterface* m_featureProcessor = nullptr;

        mutable AZ::RPI::AssetUtils::AsyncAssetLoader m_asyncAssetLoader;
        mutable bool m_asyncLoadStarted = false;
        mutable AZ::Data::Instance<AZ::RPI::StreamingImage> m_invalidParentPassTexture;

        RenderJoyTemplatesFactory m_templatesFactory;

        // We need to queue removal and addition because we have to serially and safely destroy and recreate the feature processor.
        // Key: parentPassEntityId, Value: passBusEntity
        // If passBusEntity is invalid, then we will remove parentPassEntityId.
        AZStd::map<AZ::EntityId, AZ::EntityId> m_entitiesToProcess;
    };

} // namespace RenderJoy
