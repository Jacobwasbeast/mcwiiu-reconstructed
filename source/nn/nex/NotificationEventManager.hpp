#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENTMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENTMANAGER_HPP

#include <stdint.h>
#include <list>
#include <set>

#include "nn/nex/NotificationProtocolServer.hpp"
#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/NotificationEventHandler.hpp"

namespace nn { namespace nex {

class NotificationEventManager : public _Proto_NotificationProtocolServer
{
public:
    explicit NotificationEventManager(NotificationEventManager* pParent = 0);
    virtual ~NotificationEventManager();

    virtual NotificationEventManager* Clone() const;
    virtual void ProcessNotificationEvent(const NotificationEvent& rEvent);
    virtual void ProcessNotificationEvent(unsigned int uiPrincipalID, unsigned int uiConnectionID,
                                          const NotificationEvent& rEvent);

    uint32_t RegisterNotificationEventHandler(NotificationEventHandler* pHandler);
    uint32_t UnregisterNotificationEventHandler(NotificationEventHandler* pHandler);
    uint32_t IsEventValid(const NotificationEvent& rEvent);

private:
    friend class BackEndServices;
    friend class JobWaitForNotification;
    std::set<NotificationEventHandler*> m_handlers;
    NotificationEventManager* m_pParent;
    std::list<NotificationEvent> m_events;
    unsigned int m_uiPrincipalID;
    unsigned int m_uiConnectionID;
    bool m_bDispatchServiceEvents;
};

}}

#endif
