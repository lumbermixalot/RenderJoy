/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>

#include <AzCore/Component/TickBus.h>

#include "RenderJoyKeyboardEditorComponent.h"

namespace RenderJoy
{
    void KeyboardComponentConfig::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<KeyboardComponentConfig, AZ::ComponentConfig>()
                ->Version(1)->Field(
                "ClearKeyPressedWaitTime", &KeyboardComponentConfig::m_maxWaitTimeToClearKeyPressedMilliseconds)
                ;
        }
    }

    void RenderJoyKeyboardEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        KeyboardComponentConfig::Reflect(context);

        auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<RenderJoyKeyboardEditorComponent, EditorComponentBase>()
                ->Version(1)->Field(
                "Config", &RenderJoyKeyboardEditorComponent::m_config)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext
                    ->Class<RenderJoyKeyboardEditorComponent>(
                    "RenderJoy Keyboard", "A 256x3x(1ch-uint8-unorm) image representing the keyboard state.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "RenderJoy")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Component_Placeholder.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "editor/icons/components/viewport/component_placeholder.png")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZStd::vector<AZ::Crc32>({AZ_CRC_CE("Game")}))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::HelpPageURL, "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RenderJoyKeyboardEditorComponent::m_config, "Configuration", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &RenderJoyKeyboardEditorComponent::OnConfigChanged)
                    ;

                editContext->Class<KeyboardComponentConfig>(
                    "KeyboardComponentConfig", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &KeyboardComponentConfig::m_maxWaitTimeToClearKeyPressedMilliseconds, "Clear Key Pressed Wait Time",
                        "Wait This Time In Milliseconds Before Clearing The Key Was Pressed State.")
                    ;
            }
        }

    }

    void RenderJoyKeyboardEditorComponent::Activate()
    {
        RenderJoyTextureProviderBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();
        RenderJoyKeyboardNotificationBus::Handler::BusConnect();
    }

    void RenderJoyKeyboardEditorComponent::Deactivate()
    {
        RenderJoyKeyboardNotificationBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
        RenderJoyTextureProviderBus::Handler::BusDisconnect(GetEntityId());
    }

    void RenderJoyKeyboardEditorComponent::OnConfigChanged()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    // RenderJoyKeyboardNotificationBus interface implementation
    void RenderJoyKeyboardEditorComponent::OnKeyDown(const KeyCodeInfo& keyCodeInfo)
    {
        uint8_t virtualScancode = 0xff & keyCodeInfo.m_virtualScanCode;
        const auto indexForIsKeyDown = aznumeric_cast<uint32_t>(KeyStateIndices::IndexForIsKeyDown);
        const auto indexForWasKeyPressed = aznumeric_cast<uint32_t>(KeyStateIndices::IndexForWasKeyPressed);
        if (m_textureData[indexForIsKeyDown][virtualScancode])
        {
            // In practice this never happens. But you never know.
            return;
        }
        m_textureData[indexForIsKeyDown][virtualScancode] = IsKeyDown;
        m_textureData[indexForWasKeyPressed][virtualScancode] = IsKeyDown;
        KeyAndTime keyAndTime = { virtualScancode, AZ::ScriptTimePoint(m_scriptTime.Get() + AZStd::chrono::microseconds(m_config.m_maxWaitTimeToClearKeyPressedMilliseconds * 1000)) };
        m_keysForToggleEvents.push_back(keyAndTime);

        // Notify
        AZ::RHI::Size size = GetImageSize();
        uint32_t bytesPerRow = AZ::RHI::GetFormatSize(GetPixelFormat()) * size.m_width;
        RenderJoyTextureProviderNotificationBus::Event(
            GetEntityId(), &RenderJoyTextureProviderNotification::OnPixelBufferChanged, GetPixelBuffer(), size, bytesPerRow);

        //uint8_t key = 0xff & keyCodeInfo.m_qtKeyCode;
        //uint8_t nativeScanCode = 0xff & keyCodeInfo.m_nativeScanCode;
        //AZ_TracePrintf(
        //   LogName, "OnKeyDown with key=[%c, 0x%02X], nativeScanCode=[%c, 0x%02X], virtualScancode=[%c, 0x%02X]", key,
        //   key, nativeScanCode, nativeScanCode, virtualScancode, virtualScancode);
    }

    void RenderJoyKeyboardEditorComponent::OnKeyUp(const KeyCodeInfo& keyCodeInfo)
    {
        uint8_t virtualScancode = 0xff & keyCodeInfo.m_virtualScanCode;
        const auto indexForIsKeyDown = aznumeric_cast<uint32_t>(KeyStateIndices::IndexForIsKeyDown);
        if (m_textureData[indexForIsKeyDown][virtualScancode] == IsKeyUp)
        {
            // In practice this never happens. But you never know.
            return;
        }
        m_textureData[indexForIsKeyDown][virtualScancode] = 0x00;
        const auto indexForToggle = aznumeric_cast<uint32_t>(KeyStateIndices::IndexForToggle);
        const auto toggleValue = m_textureData[indexForToggle][virtualScancode];
        m_textureData[indexForToggle][virtualScancode] = ~toggleValue;

        // Notify
        AZ::RHI::Size size = GetImageSize();
        uint32_t bytesPerRow = AZ::RHI::GetFormatSize(GetPixelFormat()) * size.m_width;
        RenderJoyTextureProviderNotificationBus::Event(
            GetEntityId(), &RenderJoyTextureProviderNotification::OnPixelBufferChanged, GetPixelBuffer(), size,
            bytesPerRow);

        //char key = 0xff & keyCodeInfo.m_qtKeyCode;
        //char nativeScanCode = 0xff & keyCodeInfo.m_nativeScanCode;
        //AZ_TracePrintf(
        //    LogName, "OnKeyUp with key=[%c, 0x%02X], nativeScanCode=[%c, 0x%02X], virtualScancode=[%c, 0x%02X]", key,
        //    key, nativeScanCode, nativeScanCode, virtualScancode, virtualScancode);
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // TickBus interface implementation
    void RenderJoyKeyboardEditorComponent::OnTick(float /*deltaTime*/, AZ::ScriptTimePoint time)
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
                        m_textureData[indexForWasKeyPressed][keyAndTime.m_virtualScanCode] = 0; 
                        notify = true;
                        return true;
                    }
                    return false;
                }),
            m_keysForToggleEvents.end());
        if (notify)
        {
            AZ::RHI::Size size = GetImageSize();
            uint32_t bytesPerRow = AZ::RHI::GetFormatSize(GetPixelFormat()) * size.m_width;
            RenderJoyTextureProviderNotificationBus::Event(
                GetEntityId(), &RenderJoyTextureProviderNotification::OnPixelBufferChanged, GetPixelBuffer(), size, bytesPerRow);
        }
    }
    //////////////////////////////////////////////////////////////////////////

} // namespace RenderJoy
