#ifndef SOURCE_PLATFORM_WIIU___N_36_INET_INETPROCESSHOSTMIGRATIONJOB_CPP_6BED3C84__NEXNOTIFICATIONEVENTHANDLER4PIA_HPP
#define SOURCE_PLATFORM_WIIU___N_36_INET_INETPROCESSHOSTMIGRATIONJOB_CPP_6BED3C84__NEXNOTIFICATIONEVENTHANDLER4PIA_HPP

#include <nex.h>
#include <pia.h>

namespace __N_36_inet_InetProcessHostMigrationJob_cpp_6bed3c84
{
class NexNotificationEventHandler4Pia : public nn::nex::NotificationEventHandler
{
public:
    virtual ~NexNotificationEventHandler4Pia();
    virtual void ProcessNotificationEvent(const nn::nex::NotificationEvent& event);

private:
    nn::pia::inet::InetProcessHostMigrationJob* m_hostMigrationJob;
};
}

#endif
