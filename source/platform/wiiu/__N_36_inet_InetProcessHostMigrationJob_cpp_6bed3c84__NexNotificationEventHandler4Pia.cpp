#include "nn/nex/NotificationEvent.hpp"
#include "nn/pia/inet/InetProcessHostMigrationJob.hpp"
#include "platform/wiiu/__N_36_inet_InetProcessHostMigrationJob_cpp_6bed3c84__NexNotificationEventHandler4Pia.hpp"

namespace __N_36_inet_InetProcessHostMigrationJob_cpp_6bed3c84
{
NexNotificationEventHandler4Pia::~NexNotificationEventHandler4Pia()
{
}

void NexNotificationEventHandler4Pia::ProcessNotificationEvent(const nn::nex::NotificationEvent& event)
{
    if (event.GetType() / 1000 != 110)
    {
        return;
    }
    m_hostMigrationJob->HostChangeEventNotice(event.GetParam2());
}
}
