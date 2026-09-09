#include "nn/nex/NotificationEventManager.hpp"
#include "nn/nex/NotificationEvent.hpp"

namespace nn { namespace nex {

NotificationEventManager::NotificationEventManager()
    : ServerProtocol(1)
    , m_handlers()
    , m_constructorContext(0)
    , m_recentEvents()
    , m_eventCount(0)
    , m_dispatchCount(0)
{
}

NotificationEventManager::NotificationEventManager(const NotificationEventManager& other)
    : ServerProtocol(1)
    , m_handlers()
    , m_constructorContext(const_cast<NotificationEventManager*>(&other))
    , m_recentEvents()
    , m_eventCount(0)
    , m_dispatchCount(0)
{
}

NotificationEventManager::~NotificationEventManager()
{
    m_handlers.clear();
    m_recentEvents.clear();
}

NotificationEventManager* NotificationEventManager::Clone() const
{
    return new NotificationEventManager(*this);
}

std::uint32_t NotificationEventManager::RegisterNotificationEventHandler(NotificationEventHandler* handler)
{
    if (handler == 0)
        return 0;
    return m_handlers.insert(handler).second ? 1u : 0u;
}

std::uint32_t NotificationEventManager::UnregisterNotificationEventHandler(NotificationEventHandler* handler)
{
    if (handler == 0)
        return 0;
    return m_handlers.erase(handler) == 1 ? 1u : 0u;
}

std::uint32_t NotificationEventManager::IsEventValid(const NotificationEvent& event)
{
    if (event.GetType() / 1000 != 4)
        return 1;
    return event.GetParam1() != 0 ? 1u : 0u;
}

void NotificationEventManager::ProcessNotificationEvent(const NotificationEvent& event)
{
    if (m_recentEvents.size() >= 5)
        m_recentEvents.pop_front();
    m_recentEvents.push_back(event);
    ++m_eventCount;
    for (std::set<NotificationEventHandler*>::iterator it = m_handlers.begin(); it != m_handlers.end(); ++it)
        (*it)->ProcessNotificationEvent(event);
}

void NotificationEventManager::ProcessNotificationEvent(std::uint32_t sender, std::uint32_t target,
                                                        const NotificationEvent& event)
{
    ++m_dispatchCount;
    (void)sender;
    (void)target;
    if (IsEventValid(event) != 0)
        ProcessNotificationEvent(event);
}

}}
