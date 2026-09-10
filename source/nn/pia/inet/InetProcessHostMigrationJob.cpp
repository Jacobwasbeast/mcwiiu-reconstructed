#include "nn/pia/inet/InetProcessHostMigrationJob.hpp"

namespace nn { namespace pia { namespace inet {

struct NexFacade
{
    char m_pad[0x44];
    unsigned int m_sessionId;
};

NexFacade* g_nexFacade = 0;

void InetProcessHostMigrationJob::HostChangeEventNotice(unsigned int sessionId)
{
    if (g_nexFacade != 0 && g_nexFacade->m_sessionId == sessionId)
    {
        m_hostChanged = true;
    }
}

}}}
