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
#include <RenderJoy/RenderJoyBillboardBus.h>

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

        enum class DisplayMode : uint32_t
        {
            Billboard,
            Flatscreen,
            MaxModes,
        };

        // Entity that owns a RenderJoy shader.
        AZ::EntityId m_shaderEntityId;

        DisplayMode m_displayMode = DisplayMode::Billboard;
        
        struct BillboardOptions
        {
            AZ_TYPE_INFO(BillboardOptions, "{E30A10C4-713E-4230-BB8A-FA4ED00E3DFD}");

            static void Reflect(AZ::ReflectContext* context);

            bool m_alwaysFaceCamera = true;

            bool operator==(const BillboardOptions& rhs) const
            {
                return m_alwaysFaceCamera == rhs.m_alwaysFaceCamera;
            }

            bool operator!=(const BillboardOptions& rhs) const
            {
                return !(*this == rhs);
            }
        };
        BillboardOptions m_billboardOptions;

        struct FlatscreenOptions
        {
            AZ_TYPE_INFO(FlatscreenOptions, "{9D9F1465-3972-40B0-839C-811634FEC39B}");

            static void Reflect(AZ::ReflectContext* context);

            // Values between 0.0 and 1.0 as proportions
            // of viewport size.
            float m_scaleX = 1.0f;
            float m_scaleY = 1.0f;
            float m_posX = 0.0f;
            float m_posY = 0.0f;
            float m_width = 1.0f;
            float m_height = 1.0f;

            bool operator==(const FlatscreenOptions& rhs) const
            {
                return (
                    AZ::IsClose(m_scaleX, rhs.m_scaleX) &&
                    AZ::IsClose(m_scaleY, rhs.m_scaleY) &&
                    AZ::IsClose(m_posX, rhs.m_posX) &&
                    AZ::IsClose(m_posY, rhs.m_posY) &&
                    AZ::IsClose(m_width, rhs.m_width) &&
                    AZ::IsClose(m_height, rhs.m_height)
                    );
            }

            bool operator!=(const FlatscreenOptions& rhs) const
            {
                return !(*this == rhs);
            }
        };
        FlatscreenOptions m_flatscreenOptions;
    };

    class RenderJoyBillboardPass;

    class RenderJoyBillboardComponentController final
        : public AZ::Data::AssetBus::MultiHandler
        , private AZ::TransformNotificationBus::Handler
        , private RenderJoyNotificationBus::Handler
        , public RenderJoyBillboardBus::Handler
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

        ///////////////////////////////////////////////////////////
        // RenderJoyBillboardBus::Handler overrides START
        void SetFlatscreenMode(float posX, float posY, float width, float height) override;
        void SetBillboardMode(bool alwaysFaceCamera) override;
        // RenderJoyBillboardBus::Handler overrides END
        ///////////////////////////////////////////////////////////

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
        void UpdateBillboardPassShaderConstants();

        // RenderJoyFeatureProcessorInterface* m_featureProcessor = nullptr;
        AZ::TransformInterface* m_transformInterface = nullptr;

        AZ::EntityId m_entityId;
        
        RenderJoyBillboardComponentConfig m_configuration;
        RenderJoyBillboardComponentConfig m_prevConfiguration;

        // Will get the real reference once we get the OnFeatureProcessorActivated notification. 
        RenderJoyBillboardPass* m_billboardPass = nullptr;

    };
} // namespace RenderJoy
