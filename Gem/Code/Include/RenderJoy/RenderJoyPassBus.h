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

        virtual bool IsOutputPass() = 0;
        virtual AZStd::vector<AZ::EntityId> GetEntitiesOnInputChannels() = 0;
        virtual AZ::Data::Asset<AZ::RPI::ShaderAsset> GetShaderAsset() = 0;
        virtual uint32_t GetRenderTargetWidth() = 0; // Only matters for render passes that are NOT the final output pass.
        virtual uint32_t GetRenderTargetHeight() = 0; // Only matters for render passes that are NOT the final output pass.
    };
    using RenderJoyPassRequestBus = AZ::EBus<RenderJoyPassRequests>;

    class RenderJoyPassNotifications
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId; // An Entity with a RenderJoyPassEditorComponent.
        //////////////////////////////////////////////////////////////////////////

        virtual void OnOutputPassChanged(bool isOutputPass) = 0;
        virtual void OnShaderAssetChanged(AZ::Data::Asset<AZ::RPI::ShaderAsset> newShaderAsset) = 0;
        virtual void OnInputChannelEntityChanged(uint32_t inputChannelIndex, AZ::EntityId newEntityId) = 0;
    };

    using RenderJoyPassNotificationBus = AZ::EBus<RenderJoyPassNotifications>;


    namespace RenderJoyPassRequestBusUtils
    {
        //! enumerates all render joy passes and it will return the last entity
        //! that claims to be the output pass.
        AZ::EntityId DiscoverOutputPass(AZ::u32& outputPassCount);

        //! Returns true if the entity is a render joy pass (Contains a component that implements RenderJoyPassRequestBus)
        bool IsRenderJoyPass(AZ::EntityId entityId);

        //! Returns true if the entity is a render joy texture provider (Contains a component that implements RenderJoyTextureProviderBus)
        bool IsRenderJoyTextureProvider(AZ::EntityId entityId);

        //! A RenderJoy pass has the exact same name as the EntityId that holds
        //! the RenderJoyPassEditorComponent, and is an entity that implements
        //! RenderJoyPassRequestBus
        AZ::EntityId GetEntityIdFromPassName(const AZ::Name& passName);

    }

} // namespace RenderJoy
