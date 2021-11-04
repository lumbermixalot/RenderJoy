
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

#include "RenderJoyDemoProjectSystemComponent.h"

namespace RenderJoyDemoProject
{
    void RenderJoyDemoProjectSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<RenderJoyDemoProjectSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<RenderJoyDemoProjectSystemComponent>("RenderJoyDemoProject", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void RenderJoyDemoProjectSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("RenderJoyDemoProjectService"));
    }

    void RenderJoyDemoProjectSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("RenderJoyDemoProjectService"));
    }

    void RenderJoyDemoProjectSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        AZ_UNUSED(required);
    }

    void RenderJoyDemoProjectSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }
    
    RenderJoyDemoProjectSystemComponent::RenderJoyDemoProjectSystemComponent()
    {
        if (RenderJoyDemoProjectInterface::Get() == nullptr)
        {
            RenderJoyDemoProjectInterface::Register(this);
        }
    }

    RenderJoyDemoProjectSystemComponent::~RenderJoyDemoProjectSystemComponent()
    {
        if (RenderJoyDemoProjectInterface::Get() == this)
        {
            RenderJoyDemoProjectInterface::Unregister(this);
        }
    }

    void RenderJoyDemoProjectSystemComponent::Init()
    {
    }

    void RenderJoyDemoProjectSystemComponent::Activate()
    {
        RenderJoyDemoProjectRequestBus::Handler::BusConnect();
    }

    void RenderJoyDemoProjectSystemComponent::Deactivate()
    {
        RenderJoyDemoProjectRequestBus::Handler::BusDisconnect();
    }
}
