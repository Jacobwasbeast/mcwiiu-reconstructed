#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNOTIFICATIONDATAJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNOTIFICATIONDATAJOB_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace inet {

class NexNotificationDataJob
{
public:
    NexNotificationDataJob();
    ~NexNotificationDataJob();
    uint64_t Startup(uint32_t a1, uint32_t a2, int32_t a3, uint32_t a4, uint32_t a5, uint32_t a6);
    void Startup(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4);
    uint32_t WaitUpdateNotificationData();
    uint32_t WaitRetrieveNotificationData();
    uint64_t Cleanup();

private:
    uint64_t state_;
};

}}}

#endif
