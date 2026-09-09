#include "nn/pia/session/Mesh.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/session/SyncClockProtocol.hpp"

namespace nn { namespace pia { namespace session {

Mesh* g_piaMesh = 0;

unsigned int Mesh::GetHostMigrationMode() const
{
    return m_hostMigrationMode;
}

bool Mesh::IsRecentPlayRecordEnabled() const
{
    return m_recentPlayRecordEnabled != 0;
}

void Mesh::SetSyncClockPulseInterval(int interval)
{
    if (transport::g_piaTransport == 0)
        return;

    transport::Protocol* protocol =
        transport::g_piaTransport->m_protocolManager.SearchProtocol(
            m_syncClockProtocolId, SyncClockProtocol::cProtocolType);
    if (protocol != 0)
        static_cast<SyncClockProtocol*>(protocol)->m_pulseInterval = interval;
}

void Mesh::SetSkipCleanupReport(bool skip)
{
    m_skipCleanupReport = skip;
}

}}}
