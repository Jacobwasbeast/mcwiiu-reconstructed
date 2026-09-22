#include "nn/pia/inet/NexNatServerAddressResolveJob.hpp"

namespace nn { namespace pia { namespace inet {

uint64_t NexNatServerAddressResolveJob::Startup(uint32_t a1, int32_t a2)
{
    return static_cast<uint64_t>(state_);
}

void NexNatServerAddressResolveJob::Cleanup()
{
    state_ += 1;
}

void NexNatServerAddressResolveJob::CancelCleanup()
{
    state_ += 1;
}

uint32_t NexNatServerAddressResolveJob::StepResolveNatServerAddress()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNatServerAddressResolveJob::StepRequestRelayServerSetting()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNatServerAddressResolveJob::StepWaitRequestRelayServerSetting()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNatServerAddressResolveJob::StepResolveRelayServerSetting()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNatServerAddressResolveJob::StepComplete()
{
    return static_cast<uint32_t>(state_);
}

}}}
