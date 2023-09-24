
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>


#include "Components/EditorRenderJoyBillboardComponent.h"
#include "Components/EditorRenderJoyShaderComponent.h"
#include "Components/EditorRenderJoyTextureComponent.h"
#include "Components/EditorRenderJoyKeyboardComponent.h"

#include "EditorRenderJoySystemComponent.h"


// #include "Components/RenderJoySettingsEditorComponent.h"

namespace RenderJoy
{
    class EditorRenderJoyModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(EditorRenderJoyModule, "{a041cb2d-dbba-4841-8c3a-0d2d50ea5c41}", AZ::Module);
        AZ_CLASS_ALLOCATOR(EditorRenderJoyModule, AZ::SystemAllocator, 0);

        EditorRenderJoyModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                RenderJoySystemComponent::CreateDescriptor(),
                EditorRenderJoySystemComponent::CreateDescriptor(),
                
                RenderJoyBillboardComponent::CreateDescriptor(),
                EditorRenderJoyBillboardComponent::CreateDescriptor(),

                RenderJoyTextureComponent::CreateDescriptor(),
                EditorRenderJoyTextureComponent::CreateDescriptor(),
                
                RenderJoyShaderComponent::CreateDescriptor(),
                EditorRenderJoyShaderComponent::CreateDescriptor(),

                RenderJoyKeyboardComponent::CreateDescriptor(),
                EditorRenderJoyKeyboardComponent::CreateDescriptor(),

                //RenderJoySettingsEditorComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<EditorRenderJoySystemComponent>(),
            };
        }
    };
}// namespace RenderJoy

AZ_DECLARE_MODULE_CLASS(Gem_RenderJoy, RenderJoy::EditorRenderJoyModule)
