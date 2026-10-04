#include "nn/pia/inet/NexNotificationDataJob.hpp"

namespace nn { namespace pia { namespace inet {

Result NexNotificationDataJob::Startup(common::CallContext* callContext, unsigned int type, unsigned int param1, unsigned int param2, const common::String& text)
{
    return 0;
}

Result NexNotificationDataJob::Startup(common::CallContext* callContext, unsigned char typeMask, nex::qList<nex::NotificationEvent>* events)
{
    return 0;
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
