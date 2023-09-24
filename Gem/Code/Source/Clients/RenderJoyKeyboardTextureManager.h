/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzCore/Script/ScriptTimePoint.h>

#include <AzFramework/Input/Channels/InputChannel.h>

#include <Atom/RPI.Public/Image/AttachmentImage.h>

namespace RenderJoy
{
    //! This class consolidates the management of the state of the keyboard
    //! into a cpu Buffer thjat is synchronized with an GPU Image and that can be used
    //! by RenderJoy Shaders as in SRV to read the state of the keyboard per the
    //! ShaderToy standards.
    //! This is a singleton owned by the RenderSystemComponent.
    class RenderJoyKeyboardTextureManager final
    {
    public:
        RenderJoyKeyboardTextureManager();
        ~RenderJoyKeyboardTextureManager() = default;

        static constexpr char LogName[] = "RenderJoyKeyboardTextureManager";

    private:
        friend class RenderJoySystemComponent;

        static constexpr uint32_t CharacterCount = 256;
        static constexpr uint8_t IsKeyDown = 0xFF;
        static constexpr uint8_t IsKeyUp = 0;
        enum class KeyStateIndices : uint32_t
        {
            IndexForIsKeyDown,
            IndexForWasKeyPressed,
            IndexForToggle,
            StateCount, // Number of states.
        };

        // Called by RenderJoySystemComponent when a keyboard event has occured. 
        void OnKeyboardChannelEvent(const AzFramework::InputChannel& inputChannel, bool& hasBeenConsumed);

        // This one is called from RenderJoySystemComponent::OnTick
        void OnTick(float deltaTime, AZ::ScriptTimePoint time);

        // Helpers
        void ResetCpuData();
        void CreateKeyboardTextureImage();

        uint8_t m_textureCpuData[aznumeric_cast<size_t>(KeyStateIndices::StateCount)][CharacterCount];
        // Only attachment images can be modified at runtime.
        AZ::Data::Instance<AZ::RPI::AttachmentImage> m_texture;
        AZ::RHI::ImageUpdateRequest m_updateRequest; // We cache here the information on how to update @m_texture.

        // Each time a key is pressed, we use this struct to store
        // the key code and the time where it was pressed.
        // Later during OnTick We check the expired ones and clear the IndexForWasKeyPressed
        // state.
        struct KeyAndTime
        {
            uint8_t m_virtualScanCode;
            AZ::ScriptTimePoint m_pressedTime;
        };
        AZ::ScriptTimePoint m_scriptTime;
        AZStd::vector<KeyAndTime> m_keysForToggleEvents;
        uint32_t m_maxWaitTimeToClearKeyPressedMilliseconds = 1000;
    };
} // namespace RenderJoy