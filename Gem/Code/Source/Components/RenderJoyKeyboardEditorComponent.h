/*
 * Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>

#include <RenderJoy/RenderJoyKeyboardBus.h>
#include <RenderJoy/RenderJoyTextureProviderBus.h>

namespace RenderJoy
{
    class KeyboardComponentConfig final
        : public AZ::ComponentConfig
    {
    public:
        AZ_RTTI(KeyboardComponentConfig, "{EB0A05C1-8E90-49C3-884E-FA1A4460F003}", AZ::ComponentConfig);
        static void Reflect(AZ::ReflectContext* context);

        uint32_t m_maxWaitTimeToClearKeyPressedMilliseconds = 0;
    };

    /*
    * Provides keyboard state data as a 256x3 (1ch, int8) texture.
    */
    class RenderJoyKeyboardEditorComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , public RenderJoyTextureProviderBus::Handler
        , public AZ::TickBus::Handler
        , public RenderJoyKeyboardNotificationBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(
            RenderJoyKeyboardEditorComponent, "{AE372E55-7FD4-4BB3-8F59-C29557A5A1AC}"
            , AzToolsFramework::Components::EditorComponentBase);

        static constexpr char LogName[] = "RenderJoyKeyboardEditorComponent";

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& services)
        {
            services.push_back(AZ::Crc32("RenderJoyKeyboardComponentService"));
        }

        static void GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& services)
        {
            services.push_back(AZ::Crc32("RenderJoyTextureComponentService"));
            services.push_back(AZ::Crc32("RenderJoyPassComponentService"));
            services.push_back(AZ::Crc32("RenderJoyKeyboardComponentService"));
        }

        static void GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& services) {}
        static void GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& services) {}

        //////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;
        //////////////////////////////////////////////////////////////////////////

        void OnConfigChanged();

    protected:

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

        //////////////////////////////////////////////////////////////////////////
        // TickBus interface implementation
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // RenderJoyTextureProviderBus overrides
        bool IsImmutable() override { return false; }
        const void* GetPixelBuffer() const override { return m_textureData; }
        AZ::RHI::Format GetPixelFormat() const override { return AZ::RHI::Format::R8_UNORM; }
        AZ::RHI::Size GetImageSize() const override
        {
            const auto width = CharacterCount;
            const uint32_t height = aznumeric_cast<uint32_t>(KeyStateIndices::StateCount);
            return AZ::RHI::Size(width, height, 1);
        }
        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////
        // RenderJoyKeyboardNotificationBus interface implementation
        void OnKeyDown(const KeyCodeInfo& keyCodeInfo) override;
        void OnKeyUp(const KeyCodeInfo& keyCodeInfo) override;
        //////////////////////////////////////////////////////////////////////////

        KeyboardComponentConfig m_config;
        uint8_t m_textureData[aznumeric_cast<size_t>(KeyStateIndices::StateCount)][CharacterCount] = {{{0}, {0}, {0}}};

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
    };
} // namespace RenderJoy
