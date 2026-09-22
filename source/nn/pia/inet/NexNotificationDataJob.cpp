#include "nn/pia/inet/NexNotificationDataJob.hpp"

namespace nn { namespace pia { namespace inet {

uint64_t NexNotificationDataJob::Startup(uint32_t a1, uint32_t a2, int32_t a3, uint32_t a4, uint32_t a5, uint32_t a6)
{
    return static_cast<uint64_t>(state_);
}

void NexNotificationDataJob::Startup(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4)
{
    state_ += 1;
}

uint32_t NexNotificationDataJob::WaitUpdateNotificationData()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNotificationDataJob::WaitRetrieveNotificationData()
{
    return static_cast<uint32_t>(state_);
}

uint64_t NexNotificationDataJob::Cleanup()
{
    return static_cast<uint64_t>(state_);
}

}}}
