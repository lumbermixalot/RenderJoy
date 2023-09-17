/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Interface/Interface.h>
#include <AzCore/Component/EntityId.h>

#include <Atom/RPI.Reflect/Image/Image.h>

namespace RenderJoy
{
    //! This is the AZ::Interface<> declaration for the singleton responsible
    //! for providing per frame data for RenderJoy shaders.
    class IKeyboardComponentsManager
    {
    public:
        AZ_RTTI(IKeyboardComponentsManager, "{B85C7B44-706B-4CDE-9D98-8CD3E3AAA2E1}");

        static constexpr const char* LogName = "IKeyboardComponentsManager";

        virtual ~IKeyboardComponentsManager() = default;

        virtual void RegisterKeyboardComponent(AZ::EntityId entityId) = 0;
        virtual void UnregisterKeyboardComponent(AZ::EntityId entityId) = 0;

        virtual void UpdateClearKeyPressedMilliseconds(uint32_t milliSeconds) = 0;
        virtual AZ::Data::Instance<AZ::RPI::Image> GetKeyboardTexture() = 0;

    };

    using RenderJoyKeyboardManagerInterface = AZ::Interface<IKeyboardComponentsManager>;

} // namespace RenderJoy
