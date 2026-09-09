#include "nn/nex/NotificationEventManager.hpp"

#include "nn/nex/String.hpp"

namespace nn { namespace nex {

NotificationEventManager::NotificationEventManager(NotificationEventManager* pParent)
    : _Proto_NotificationProtocolServer(1)
    , m_handlers()
    , m_pParent(pParent)
    , m_events()
    , m_uiPrincipalID(0)
    , m_uiConnectionID(0)
{
    SetFlag(8);
    SetName(String(L"NotificationEventManager"));
}

NotificationEventManager::~NotificationEventManager()
{
    m_handlers.clear();
    m_events.clear();
}

NotificationEventManager* NotificationEventManager::Clone() const
{
    return new NotificationEventManager(const_cast<NotificationEventManager*>(this));
}

std::uint32_t NotificationEventManager::RegisterNotificationEventHandler(NotificationEventHandler* pHandler)
{
    if (pHandler == 0)
        return 0;
    return m_handlers.insert(pHandler).second ? 1u : 0u;
}

std::uint32_t NotificationEventManager::UnregisterNotificationEventHandler(NotificationEventHandler* pHandler)
{
    if (pHandler == 0)
        return 0;
    return m_handlers.erase(pHandler) == 1 ? 1u : 0u;
}

std::uint32_t NotificationEventManager::IsEventValid(const NotificationEvent& rEvent)
{
    if (rEvent.GetType() / 1000 != 4)
        return 1;
    return rEvent.GetParam1() != 0 ? 1u : 0u;
}

void NotificationEventManager::ProcessNotificationEvent(const NotificationEvent&)
{
}

void NotificationEventManager::ProcessNotificationEvent(unsigned int uiPrincipalID,
                                                        unsigned int uiConnectionID,
                                                        const NotificationEvent& rEvent)
{
    if (uiPrincipalID != m_uiPrincipalID || uiConnectionID != m_uiConnectionID)
        return;
    if (IsEventValid(rEvent) == 0)
        return;

    for (std::set<NotificationEventHandler*>::iterator it = m_handlers.begin();
         it != m_handlers.end(); ++it)
    {
        if (rEvent.GetType() / 1000 != 115 || m_bDispatchServiceEvents)
            (*it)->ProcessNotificationEvent(rEvent);
    }
}

}}
