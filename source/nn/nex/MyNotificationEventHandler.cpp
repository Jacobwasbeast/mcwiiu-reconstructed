#include "nn/nex/MyNotificationEventHandler.hpp"

namespace nn { namespace nex {

MyNotificationEventHandler::MyNotificationEventHandler(std::uint32_t eventGroup, std::uint32_t eventType)
    : m_eventReady(false)
    , m_hasParam1(false)
    , m_hasUInt64Param1(false)
    , m_eventGroup(eventGroup)
    , m_eventType(eventType)
    , m_param1(0)
    , m_uint64Param1(0)
    , m_pendingEvents()
    , m_event()
{
}

MyNotificationEventHandler::~MyNotificationEventHandler()
{
}

void MyNotificationEventHandler::SetEventParam1(std::uint32_t value)
{
    m_param1 = value;
    m_hasParam1 = true;
    m_eventReady = false;
    m_pendingEvents.clear();
}

void MyNotificationEventHandler::SetEventUInt64Param1(std::uint64_t value)
{
    m_uint64Param1 = value;
    m_hasUInt64Param1 = true;
    m_eventReady = false;
    m_pendingEvents.clear();
}

void MyNotificationEventHandler::ProcessNotificationEvent(const NotificationEvent& event)
{
    if (event.GetType() / 1000 != m_eventGroup || event.GetType() % 1000 != m_eventType)
        return;

    if (!m_hasParam1 && !m_hasUInt64Param1)
    {
        m_pendingEvents.push_back(event);
        if (m_pendingEvents.size() > 16)
            m_pendingEvents.erase(m_pendingEvents.begin());
        return;
    }

    if (m_hasParam1 && event.GetParam1() != m_param1)
        return;
    if (m_hasUInt64Param1 && (event.GetParam2() != static_cast<std::uint32_t>(m_uint64Param1 >> 32) ||
                              event.GetParam3() != static_cast<std::uint32_t>(m_uint64Param1)))
        return;

    m_event = event;
    m_eventReady = true;
}

bool MyNotificationEventHandler::HasEvent() const
{
    return m_eventReady;
}

const NotificationEvent& MyNotificationEventHandler::GetEvent() const
{
    return m_event;
}

}}
