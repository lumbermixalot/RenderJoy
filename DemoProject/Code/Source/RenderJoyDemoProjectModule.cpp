
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

#include "RenderJoyDemoProjectSystemComponent.h"

namespace RenderJoyDemoProject
{
    class RenderJoyDemoProjectModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(RenderJoyDemoProjectModule, "{cc218e9a-81c0-4864-bf1f-b29fe8fbef2f}", AZ::Module);
        AZ_CLASS_ALLOCATOR(RenderJoyDemoProjectModule, AZ::SystemAllocator, 0);

        RenderJoyDemoProjectModule()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                RenderJoyDemoProjectSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<RenderJoyDemoProjectSystemComponent>(),
            };
        }
    };
}// namespace RenderJoyDemoProject

AZ_DECLARE_MODULE_CLASS(Gem_RenderJoyDemoProject, RenderJoyDemoProject::RenderJoyDemoProjectModule)
