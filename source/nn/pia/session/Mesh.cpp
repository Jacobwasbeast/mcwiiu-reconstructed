#include "nn/pia/session/Mesh.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/session/KickoutManageJob.hpp"
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

Result Mesh::CheckConnectionError() const
{
    if (m_meshState == 1)
        return 0;
    return static_cast<Result>(0x6c50);
}

unsigned int Mesh::GetDisconnectReason() const
{
    if (m_meshState == 1)
        return 0;
    if (m_disconnectReason == 1)
        return 1;
    if (m_kickoutManageJob->m_kickoutReason == 0)
        return m_disconnectReason;
    if (m_kickoutManageJob->m_kickoutReason != 2)
        return 5;
    return 4;
}

KickoutManageJob* Mesh::GetKickoutManageJobPtr()
{
    return m_kickoutManageJob;
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
