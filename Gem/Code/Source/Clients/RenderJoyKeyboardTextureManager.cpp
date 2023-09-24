/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboardWindowsScanCodes.h>

#include <Atom/RPI.Public/Image/AttachmentImagePool.h>
#include <Atom/RPI.Public/Image/ImageSystemInterface.h>

#include "RenderJoyKeyboardTextureManager.h"

namespace RenderJoy
{
    void RenderJoyKeyboardTextureManager::CreateKeyboardTextureImage()
    {
        const AZ::RHI::Format pixFormat = AZ::RHI::Format::R8_UNORM;
        const uint32_t width = CharacterCount;
        const uint32_t height = static_cast<uint32_t>(KeyStateIndices::StateCount);
        const uint32_t bytesPerRow = width * AZ::RHI::GetFormatSize(pixFormat);
        const AZStd::string imageName("RenderJoy_Keyboard");

        AZ::Data::Instance<AZ::RPI::AttachmentImagePool> pool = AZ::RPI::ImageSystemInterface::Get()->GetSystemAttachmentPool();
        auto imageDesc = AZ::RHI::ImageDescriptor::Create2D(AZ::RHI::ImageBindFlags::ShaderRead, width, height, pixFormat);
        AZ::RHI::ClearValue clearValue = AZ::RHI::ClearValue::CreateVector4Float(0, 0, 0, 0);

        m_texture =  AZ::RPI::AttachmentImage::Create(*pool.get(), imageDesc, AZ::Name(imageName), &clearValue, nullptr);

        // Cache the update request, which will be the same each time a keyboard button changes state.
        AZ::RHI::ImageUpdateRequest updateRequest;
        AZ::RHI::Size imageSize(width, height, 1);
        m_updateRequest.m_image = m_texture->GetRHIImage();
        m_updateRequest.m_sourceData = m_textureCpuData;
        m_updateRequest.m_sourceSubresourceLayout.m_size = imageSize;
        m_updateRequest.m_sourceSubresourceLayout.m_rowCount = height;
        m_updateRequest.m_sourceSubresourceLayout.m_bytesPerRow = bytesPerRow;
        m_updateRequest.m_sourceSubresourceLayout.m_bytesPerImage = bytesPerRow * height;
    }

    RenderJoyKeyboardTextureManager::RenderJoyKeyboardTextureManager()
    {
        ResetCpuData();
        CreateKeyboardTextureImage();
    }

    static uint8_t GetVirtualScanCode(const AzFramework::InputChannel& inputChannel)
    {
        // We are doing a linear search on AZStd::array because for small arrays it's potentially
        // faster than AZStd::unordered_map lookup.
        for (size_t i = 0; i < InputChannelIdByVirtualKeyCodeTable.size(); i++)
        {
            const auto* cInputChannelId = InputChannelIdByVirtualKeyCodeTable[i];
            if (!cInputChannelId)
            {
                continue;
            }
            if (*cInputChannelId == inputChannel.GetInputChannelId())
            {
                return static_cast<uint8_t>(i);
            }
        }
        return 0;
    }

    void RenderJoyKeyboardTextureManager::OnKeyboardChannelEvent(const AzFramework::InputChannel& inputChannel, [[maybe_unused]] bool& hasBeenConsumed)
    {
        constexpr auto indexForIsKeyDown = aznumeric_cast<uint32_t>(KeyStateIndices::IndexForIsKeyDown);
        const auto state = inputChannel.GetState();
        if (state == AzFramework::InputChannel::State::Began)
        {
            constexpr auto indexForWasKeyPressed = aznumeric_cast<uint32_t>(KeyStateIndices::IndexForWasKeyPressed);
            const auto virtualScancode = GetVirtualScanCode(inputChannel);
            m_textureCpuData[indexForIsKeyDown][virtualScancode] = IsKeyDown;
            m_textureCpuData[indexForWasKeyPressed][virtualScancode] = IsKeyDown;
            KeyAndTime keyAndTime = { virtualScancode, AZ::ScriptTimePoint(m_scriptTime.Get() + AZStd::chrono::microseconds(m_maxWaitTimeToClearKeyPressedMilliseconds * 1000)) };
            m_keysForToggleEvents.push_back(keyAndTime);
        }
        else if (state == AzFramework::InputChannel::State::Ended)
        {
            constexpr auto indexForToggle = aznumeric_cast<uint32_t>(KeyStateIndices::IndexForToggle);
            const auto virtualScancode = GetVirtualScanCode(inputChannel);
            m_textureCpuData[indexForIsKeyDown][virtualScancode] = IsKeyUp;
            const auto toggleValue = m_textureCpuData[indexForToggle][virtualScancode];
            m_textureCpuData[indexForToggle][virtualScancode] = ~toggleValue;
        }
        else
        {
            return;
        }

        // Update the GPU::Buffer.
        m_texture->UpdateImageContents(m_updateRequest);
    }

    void RenderJoyKeyboardTextureManager::OnTick([[maybe_unused]] float deltaTime, AZ::ScriptTimePoint time)
    {
        // The reason we use OnTick is to clear the WasKeyPressedIndex state after a short while(per ShaderToy semantics).
        const auto indexForWasKeyPressed = aznumeric_cast<uint32_t>(KeyStateIndices::IndexForWasKeyPressed);
        bool notify = false;
        m_scriptTime = time;
        m_keysForToggleEvents.erase(
            AZStd::remove_if(
                m_keysForToggleEvents.begin(), m_keysForToggleEvents.end(),
                [&](const auto & keyAndTime) {
                    if (keyAndTime.m_pressedTime.GetSeconds() <= time.GetSeconds())
                    {
                        m_textureCpuData[indexForWasKeyPressed][keyAndTime.m_virtualScanCode] = 0; 
                        notify = true;
                        return true;
                    }
                    return false;
                }),
            m_keysForToggleEvents.end());
        if (notify)
        {
            // Update the GPU::Buffer.
            m_texture->UpdateImageContents(m_updateRequest);
        }
    }

    void RenderJoyKeyboardTextureManager::ResetCpuData()
    {
        for (int i = 0; i < aznumeric_cast<int>(KeyStateIndices::StateCount); i++)
        {
            for (int j = 0; j < CharacterCount; j++)
            {
                m_textureCpuData[i][j] = 0;
            }
        }
    }


} // namespace RenderJoy