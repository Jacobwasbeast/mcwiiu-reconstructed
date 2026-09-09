#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MYNOTIFICATIONEVENTHANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MYNOTIFICATIONEVENTHANDLER_HPP

#include <cstdint>
#include <vector>

#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/NotificationEventHandler.hpp"

namespace nn { namespace nex {

class MyNotificationEventHandler : public NotificationEventHandler
{
public:
    MyNotificationEventHandler(std::uint32_t eventGroup, std::uint32_t eventType);
    ~MyNotificationEventHandler() override;

    void ProcessNotificationEvent(const NotificationEvent& event) override;
    void SetEventParam1(std::uint32_t value);
    void SetEventUInt64Param1(std::uint64_t value);

    bool HasEvent() const;
    const NotificationEvent& GetEvent() const;

private:
    bool m_eventReady;
    bool m_hasParam1;
    bool m_hasUInt64Param1;
    std::uint32_t m_eventGroup;
    std::uint32_t m_eventType;
    std::uint32_t m_param1;
    std::uint64_t m_uint64Param1;
    std::vector<NotificationEvent> m_pendingEvents;
    NotificationEvent m_event;
};

}}

#endif
