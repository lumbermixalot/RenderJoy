/*
* Copyright (c) Galib Arrieta (aka 'lumbermixalot@github', aka 'galibzon@github').
* For complete copyright and license terms please see the LICENSE at the root of this distribution.
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <Atom/Feature/Utils/EditorRenderComponentAdapter.h>

#include <AzToolsFramework/API/ComponentEntitySelectionBus.h>
#include <AzToolsFramework/Entity/EditorEntityInfoBus.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentAdapter.h>
#include <Components/RenderJoyShaderComponent.h>

#include <Atom/RPI.Public/Pass/AttachmentReadback.h>

#include <RenderJoy/RenderJoyTypeIds.h>
#include <RenderJoy/RenderJoyBus.h>
#include <Tools/Utils/ITextureWriter.h>

namespace RenderJoy
{
    class RenderJoyShaderPass;

    class SaveToDiskConfig : public AZ::ComponentConfig
    {
    public:
        AZ_CLASS_ALLOCATOR(SaveToDiskConfig, AZ::SystemAllocator);
        AZ_RTTI(SaveToDiskConfig, "{C7EB291F-6708-4931-B38F-08399D561CAD}", AZ::ComponentConfig);
        static void Reflect(AZ::ReflectContext* context);

        AZ::IO::Path m_outputImagePath;

        static AZStd::string GetSupportedImagesFilter()
        {
            return "Images (*.png *.dds)";
        }
    };

    inline constexpr AZ::TypeId EditorRenderJoyShaderComponentTypeId { "{D6CAD35B-00A1-4E33-894F-0F8BD4DD682A}" };

    class EditorRenderJoyShaderComponent final
        : public AZ::Render::EditorRenderComponentAdapter<RenderJoyShaderComponentController, RenderJoyShaderComponent, RenderJoyShaderComponentConfig>
        , private AzToolsFramework::EditorComponentSelectionRequestsBus::Handler
        , private AzToolsFramework::EditorEntityInfoNotificationBus::Handler
        , RenderJoyNotificationBus::Handler
    {
    public:
        using BaseClass = AZ::Render::EditorRenderComponentAdapter <RenderJoyShaderComponentController, RenderJoyShaderComponent, RenderJoyShaderComponentConfig>;
        AZ_EDITOR_COMPONENT(EditorRenderJoyShaderComponent, EditorRenderJoyShaderComponentTypeId, BaseClass);

        static void Reflect(AZ::ReflectContext* context);

        EditorRenderJoyShaderComponent();
        EditorRenderJoyShaderComponent(const RenderJoyShaderComponentConfig& config);

        static constexpr char LogName[] = "EditorRenderJoyShaderComponent";

    private:

        // AZ::Component overrides
        void Activate() override;
        void Deactivate() override;

        // EditorComponentAdapter overrides;
        AZ::u32 OnConfigurationChanged() override;

        // Captures the current rendered image and saves it to disk.
        AZ::u32 OnSaveToDisk();
        // Helper function that makes the SaveToDisk button disabled or enabled.
        bool IsSaveToDiskDisabled();

        void OnEntityVisibilityChanged(bool visibility) override;

        SaveToDiskConfig m_saveToDiskConfig;
        // We use this pass only to make render target capture requests.
        // Will get the real reference once we get the OnFeatureProcessorActivated notification.
        RenderJoyShaderPass* m_shaderPass = nullptr;
        AZStd::shared_ptr<AZ::RPI::AttachmentReadback> m_attachmentReadback;
        uint32_t m_readbackTaskId;
        // If this smart ptr is different than null it means we are capturing and saving the rendered texture.
        // to disk.
        AZStd::unique_ptr<ITextureWriter> m_textureWriter;
        void AttachmentReadbackCallback(const AZ::RPI::AttachmentReadback::ReadbackResult& result);
        // This one is enqueued on main thread whether saving the readback was successful or not.
        void OnSavingToDiskCompleted(bool success, AZStd::string message);

        //RenderJoyShaderComponentController::FeatureProcessorEvent::Handler m_fpActivationHandler;
        void OnFeatureProcessorActivated() override;
        void OnFeatureProcessorDeactivated() override;

    };
}
