
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace RenderJoyDemoProject
{
    class RenderJoyDemoProjectRequests
    {
    public:
        AZ_RTTI(RenderJoyDemoProjectRequests, "f47b7676-6f5c-46b7-82ec-f1972e369799");
        virtual ~RenderJoyDemoProjectRequests() = default;
        // Put your public methods here
    };
    
    class RenderJoyDemoProjectBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using RenderJoyDemoProjectRequestBus = AZ::EBus<RenderJoyDemoProjectRequests, RenderJoyDemoProjectBusTraits>;
    using RenderJoyDemoProjectInterface = AZ::Interface<RenderJoyDemoProjectRequests>;

} // namespace RenderJoyDemoProject
