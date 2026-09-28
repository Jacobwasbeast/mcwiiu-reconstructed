#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MYNOTIFICATIONEVENTHANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MYNOTIFICATIONEVENTHANDLER_HPP

#include <stdint.h>
#include <vector>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/NotificationEventHandler.hpp"

namespace nn { namespace nex {

class MyNotificationEventHandler : public NotificationEventHandler
{
public:
    MyNotificationEventHandler(uint32_t eventGroup, uint32_t eventType);
    ~MyNotificationEventHandler();

    void ProcessNotificationEvent(const NotificationEvent& event);
    void SetEventParam1(uint32_t value);
    void SetEventUInt64Param1(uint64_t value);

    bool HasEvent() const;
    const NotificationEvent& GetEvent() const;

private:
    bool m_eventReady;
    bool m_hasParam1;
    bool m_hasUInt64Param1;
    uint32_t m_eventGroup;
    uint32_t m_eventType;
    uint32_t m_param1;
    uint64_t m_uint64Param1;
    std::vector<NotificationEvent, MemAllocator<NotificationEvent> > m_pendingEvents;
    NotificationEvent m_event;
};

}}

#endif
