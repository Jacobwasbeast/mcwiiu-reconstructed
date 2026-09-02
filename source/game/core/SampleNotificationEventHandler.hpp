#ifndef GAME_CORE_SAMPLENOTIFICATIONEVENTHANDLER_HPP
#define GAME_CORE_SAMPLENOTIFICATIONEVENTHANDLER_HPP

#include <nex.h>

class SampleNotificationEventHandler : public nn::nex::NotificationEventHandler
{
public:
    virtual ~SampleNotificationEventHandler();

    virtual void ProcessNotificationEvent(const nn::nex::NotificationEvent& oEvent);
};

#endif
