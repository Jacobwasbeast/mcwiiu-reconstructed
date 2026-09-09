#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENTMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENTMANAGER_HPP

#include <cstdint>
#include <list>
#include <set>

#include "nn/nex/ServerProtocol.hpp"
#include "nn/nex/NotificationEventHandler.hpp"

namespace nn { namespace nex {

class NotificationEvent;

class NotificationEventManager : public ServerProtocol
{
public:
    NotificationEventManager();
    NotificationEventManager(const NotificationEventManager& other);
    virtual ~NotificationEventManager();

    virtual NotificationEventManager* Clone() const;
    std::uint32_t RegisterNotificationEventHandler(NotificationEventHandler* handler);
    std::uint32_t UnregisterNotificationEventHandler(NotificationEventHandler* handler);
    std::uint32_t IsEventValid(const NotificationEvent& event);
    void ProcessNotificationEvent(const NotificationEvent& event);
    void ProcessNotificationEvent(std::uint32_t sender, std::uint32_t target,
                                  const NotificationEvent& event);

private:
    std::set<NotificationEventHandler*> m_handlers;
    void* m_constructorContext;
    std::list<NotificationEvent> m_recentEvents;
    std::uint32_t m_eventCount;
    std::uint32_t m_dispatchCount;
};

}}

#endif
