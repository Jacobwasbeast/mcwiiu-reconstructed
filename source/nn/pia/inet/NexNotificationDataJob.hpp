#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNOTIFICATIONDATAJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNOTIFICATIONDATAJOB_HPP

#include <stdint.h>
#include "nn/pia/Common.hpp"

namespace nn { namespace nex { template <typename T> class qList; class NotificationEvent; }}
namespace nn { namespace pia { namespace common { class CallContext; class String; }}}

namespace nn { namespace pia { namespace inet {

class NexNotificationDataJob
{
public:
    NexNotificationDataJob();
    ~NexNotificationDataJob();
    Result Startup(common::CallContext* callContext, unsigned int type, unsigned int param1, unsigned int param2, const common::String& text);
    Result Startup(common::CallContext* callContext, unsigned char typeMask, nex::qList<nex::NotificationEvent>* events);
    uint32_t WaitUpdateNotificationData();
    uint32_t WaitRetrieveNotificationData();
    uint64_t Cleanup();

private:
    uint64_t state_;
};

}}}

#endif
