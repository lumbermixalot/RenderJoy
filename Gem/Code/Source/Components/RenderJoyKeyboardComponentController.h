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

#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>

#include <RenderJoy/RenderJoyCommon.h>
#include <RenderJoy/RenderJoyTextureProviderBus.h>
#include <RenderJoy/RenderJoyFeatureProcessorInterface.h>

namespace RenderJoy
{
    class RenderJoyKeyboardComponentConfig final
        : public AZ::ComponentConfig
    {
    public:
        AZ_RTTI(RenderJoyKeyboardComponentConfig, "{EB0A05C1-8E90-49C3-884E-FA1A4460F003}", ComponentConfig);
        AZ_CLASS_ALLOCATOR(RenderJoyKeyboardComponentConfig, AZ::SystemAllocator);
        static void Reflect(AZ::ReflectContext* context);

        RenderJoyKeyboardComponentConfig() = default;

        uint32_t m_maxWaitTimeToClearKeyPressedMilliseconds = 1000;
    };

    class RenderJoyKeyboardComponentController final
        : public RenderJoyTextureProviderBus::Handler
    {
    public:
        friend class EditorRenderJoyKeyboardComponent;

        AZ_RTTI(RenderJoyKeyboardComponentController, "{AE372E55-7FD4-4BB3-8F59-C29557A5A1AC}");
        AZ_CLASS_ALLOCATOR(RenderJoyKeyboardComponentController, AZ::SystemAllocator);

        static void Reflect(AZ::ReflectContext* context);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        RenderJoyKeyboardComponentController() = default;
        RenderJoyKeyboardComponentController(const RenderJoyKeyboardComponentConfig& config);

        void Activate(AZ::EntityId entityId);
        void Deactivate();
        void SetConfiguration(const RenderJoyKeyboardComponentConfig& config);
        const RenderJoyKeyboardComponentConfig& GetConfiguration() const;

        //////////////////////////////////////////////////////////////////////////
        // RenderJoyTextureProviderBus overrides START
        AZ::Data::Instance<AZ::RPI::Image> GetImage() const override;
        /// RenderJoyTextureProviderBus overrides END
        /////////////////////////////////////////////////////////////////

    private:

        AZ_DISABLE_COPY(RenderJoyKeyboardComponentController);

        static constexpr char LogName[] = "RenderJoyKeyboardComponentController";

        void OnConfigurationChanged();

        AZ::EntityId m_entityId;
        
        RenderJoyKeyboardComponentConfig m_configuration;
        RenderJoyKeyboardComponentConfig m_prevConfiguration;
    };
} // namespace RenderJoy
