#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATSERVERADDRESSRESOLVEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATSERVERADDRESSRESOLVEJOB_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace common {
class CallContext;
}}}

namespace nn { namespace pia { namespace inet {

class NexNatServerAddressResolveJob
{
public:
    NexNatServerAddressResolveJob();
    ~NexNatServerAddressResolveJob();
    uint64_t Startup(common::CallContext* a1);
    void Cleanup();
    void CancelCleanup();
    uint32_t StepResolveNatServerAddress();
    uint32_t StepRequestRelayServerSetting();
    uint32_t StepWaitRequestRelayServerSetting();
    uint32_t StepResolveRelayServerSetting();
    uint32_t StepComplete();

private:
    uint64_t state_;
};

}}}

#endif
