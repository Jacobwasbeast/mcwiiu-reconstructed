#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENTHANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NOTIFICATIONEVENTHANDLER_HPP

namespace nn { namespace nex {

class NotificationEvent;

class NotificationEventHandler
{
public:
    virtual ~NotificationEventHandler();
    virtual void ProcessNotificationEvent(const NotificationEvent& event) = 0;
};

}}

#endif
