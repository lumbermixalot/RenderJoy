/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Component/EntityId.h>
#include <Atom/RPI.Reflect/Shader/ShaderAsset.h>

namespace RenderJoy
{
    class RenderJoyPassRequests
        : public AZ::EBusTraits
    {
    public:
        // AZ_RTTI required on this EBUS, this allows us to iterate through the handlers of this EBUS and deduce their type.
        AZ_RTTI(RenderJoyPassRequests, "{D2B9406C-8065-4100-9689-EFCC8BA95B0C}");

        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId; // An Entity with a RenderJoyPassEditorComponent.
        //////////////////////////////////////////////////////////////////////////

        virtual AZStd::vector<AZ::EntityId> GetEntitiesOnInputChannels() const = 0;
        virtual AZ::Data::Asset<AZ::RPI::ShaderAsset> GetShaderAsset() const = 0;
        virtual uint32_t GetRenderTargetWidth() const = 0;
        virtual uint32_t GetRenderTargetHeight() const = 0;
        virtual AZ::RHI::Format GetRenderTargetFormat() const = 0;
    };
    using RenderJoyPassRequestBus = AZ::EBus<RenderJoyPassRequests>;

    namespace Utils
    {
        //! Returns true if the entity is a render joy pass (Contains a component that implements RenderJoyPassRequests)
        bool IsRenderJoyPass(AZ::EntityId entityId);
    }

    class RenderJoyPassNotifications
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId; // An Entity with a RenderJoyPassEditorComponent.
        //////////////////////////////////////////////////////////////////////////
    
        virtual void OnShaderAssetChanged(AZ::Data::Asset<AZ::RPI::ShaderAsset> newShaderAsset) = 0;
        // Typically @newEntityId is an SRV texture provider.
        virtual void OnInputChannelEntityChanged(uint32_t inputChannelIndex, AZ::EntityId newEntityId) = 0;
    };
    
    using RenderJoyPassNotificationBus = AZ::EBus<RenderJoyPassNotifications>;

} // namespace RenderJoy
