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
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>

namespace RenderJoy
{
    class RenderJoyTextureProvider
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        typedef AZ::EntityId BusIdType; // The EntityId that provides the texture
        //////////////////////////////////////////////////////////////////////////

        //! If it returns FALSE, then GetStreamingImageAsset() should return null
        //! - Immutable means that the pixel data won't change, but the whole asset could still change.
        //!   When the asset changes, listeners of RenderJoyTextureProviderNotification will get OnStreamingImageAssetChanged().
        //! - Mutable means that the pixel data can change, callers should use GetPixelBuffer
        //!   to get the CPU data. Also, when pixel data changes , listeners of RenderJoyTextureProviderNotification will get OnPixelBufferChanged().
        virtual bool IsImmutable() = 0;

        //! Immutable providers returns a valid asset. Mutable providers return null.
        virtual AZ::Data::Asset<AZ::RPI::StreamingImageAsset> GetStreamingImageAsset() const { return {}; }

        //! Mutable providers return a buffer.
        virtual const void * GetPixelBuffer() const { return nullptr; }

        //! Must be a valid format for both mutable and immutable providers.
        virtual AZ::RHI::Format GetPixelFormat() const = 0;

        //! Must be a non-zero size for both mutable and immutable providers.
        virtual AZ::RHI::Size GetImageSize() const = 0;

    };
    using RenderJoyTextureProviderBus = AZ::EBus<RenderJoyTextureProvider>;

    class RenderJoyTextureProviderNotification
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        typedef AZ::EntityId BusIdType; // The EntityId that provides the texture
        //////////////////////////////////////////////////////////////////////////

        //! The asset may be invalid/null, in such case the provider is being destroyed/deactivated.
        virtual void OnStreamingImageAssetChanged(AZ::Data::Asset<AZ::RPI::StreamingImageAsset> imageAsset) = 0;

        //! The given pointer may be nullptr, in such case the provider is being destroyed/deactivated.
        virtual void OnPixelBufferChanged(const void* pixels, AZ::RHI::Size imageSize, uint32_t bytesPerRow) = 0;

    };
    using RenderJoyTextureProviderNotificationBus = AZ::EBus<RenderJoyTextureProviderNotification>;

    namespace Utils
    {
        // Returns true is the entity implements the RenderJoyTextureProviderBus.
        bool IsRenderJoyTextureProvider(AZ::EntityId entityId);
    }
} // namespace RenderJoy
