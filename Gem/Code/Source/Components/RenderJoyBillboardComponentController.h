/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TransformBus.h>

#include <RenderJoy/RenderJoyBus.h>

//#include <RenderJoy/RenderJoyFeatureProcessorInterface.h>

namespace RenderJoy
{
    class RenderJoyBillboardComponentConfig final
        : public AZ::ComponentConfig
    {
    public:
        AZ_RTTI(RenderJoyBillboardComponentConfig, "{96E4D46A-0ABC-4361-93AA-B480C86955D4}", ComponentConfig);
        AZ_CLASS_ALLOCATOR(RenderJoyBillboardComponentConfig, AZ::SystemAllocator);
        static void Reflect(AZ::ReflectContext* context);

        RenderJoyBillboardComponentConfig() = default;

        // Entity that owns a RenderJoy shader.
        AZ::EntityId m_shaderEntityId;

        bool m_alwaysFaceCamera = false;

    };

    class RenderJoyBillboardPass;

    class RenderJoyBillboardComponentController final
        : public AZ::Data::AssetBus::MultiHandler
        , private AZ::TransformNotificationBus::Handler
        , private RenderJoyNotificationBus::Handler
    {
    public:
        friend class EditorRenderJoyBillboardComponent;

        AZ_RTTI(RenderJoyBillboardComponentController, "{6946E96C-5C63-482D-B0EC-5DE79E1771D5}");
        AZ_CLASS_ALLOCATOR(RenderJoyBillboardComponentController, AZ::SystemAllocator);

        static void Reflect(AZ::ReflectContext* context);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        RenderJoyBillboardComponentController() = default;
        RenderJoyBillboardComponentController(const RenderJoyBillboardComponentConfig& config);

        void Activate(AZ::EntityId entityId);
        void Deactivate();
        void SetConfiguration(const RenderJoyBillboardComponentConfig& config);
        const RenderJoyBillboardComponentConfig& GetConfiguration() const;

    private:

        AZ_DISABLE_COPY(RenderJoyBillboardComponentController);

        static constexpr char LogName[] = "RenderJoyBillboardComponentController";

        // TransformNotificationBus overrides
        void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;

        ///////////////////////////////////////////////////////////
        // RenderJoyNotificationBus::Handler overrides START
        void OnFeatureProcessorActivated() override;
        void OnFeatureProcessorDeactivated() override;
        // RenderJoyNotificationBus::Handler overrides END
        ///////////////////////////////////////////////////////////

        void OnConfigurationChanged();

        // RenderJoyFeatureProcessorInterface* m_featureProcessor = nullptr;
        AZ::TransformInterface* m_transformInterface = nullptr;

        AZ::EntityId m_entityId;
        
        RenderJoyBillboardComponentConfig m_configuration;
        RenderJoyBillboardComponentConfig m_prevConfiguration;

        // Will get the real reference once we get the OnFeatureProcessorActivated notification. 
        RenderJoyBillboardPass* m_billboardPass = nullptr;

    };
} // namespace RenderJoy
