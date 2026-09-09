#include "nn/pia/session/Session.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/SessionStatusCheckJob.hpp"
#include "nn/pia/session/SessionSupport.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
unsigned char s_checkStartupSettingRange = 1;
}

Session* g_piaSession = 0;
unsigned char g_piaMatchmakeAnalysisEnabled = 0;

Result Session::Startup(const StartupSetting& setting)
{
    if (IsDuringSetup() != 0)
        return static_cast<Result>(0x10c08);
    if (m_startupState != 0)
        return static_cast<Result>(0x10c08);
    if (s_checkStartupSettingRange != 0 && setting.m_field00 - 1000 > 28992)
        return static_cast<Result>(0x10c07);

    Result result = m_meshLayerController->Startup(setting);
    if (result != 0)
        return result;

    m_meshLayerController->IsHostMigrationEnabled();
    m_playerNum = 0;
    m_startupState = 1;
    m_participantNum = 0;
    m_sessionDisconnectReason = 0;

    result = m_sessionStatusCheckJob->Startup();
    if (result != 0)
        return result;

    m_sessionStatusCheckJob->Ready(false);
    m_disconnectStatus = 0;

    common::ListNode* sentinel = m_validStationIdList.Sentinel();
    common::ListNode* node = sentinel->next;
    while (node != sentinel)
    {
        common::ListNode* next = node->next;
        node->prev = 0;
        node->next = 0;
        m_validStationIdFreeList.InsertAfterNode(
            m_validStationIdFreeList.Sentinel(),
            reinterpret_cast<common::ListNode*>(
                reinterpret_cast<char*>(node) + m_stationIdNodeOffset));
        node = next;
    }
    m_validStationIdList.Init();

    g_piaMesh->SetSkipCleanupReport(true);
    return transport::g_piaTransport->Startup(m_meshLayerController->m_payloadSize, 0);
}

bool Session::IsHost() const
{
    if (m_localStationId == 0)
        return false;
    if (m_localStationId != m_hostStationId)
        return false;

    const IMatchmakeSession* matchmakeSession = m_matchmakeSession[m_currentSessionIndex];
    if (matchmakeSession == 0)
        return false;
    return matchmakeSession->IsMatchmakeOwner();
}

bool Session::IsJointSessionHost() const
{
    if (m_jointSessionJob == 0)
        return false;
    if (m_localStationId == 0)
        return false;
    if (m_localStationId != m_hostStationId)
        return false;

    const IMatchmakeSession* currentSession = m_matchmakeSession[m_currentSessionIndex];
    if (currentSession == 0 || !currentSession->IsMatchmakeOwner())
        return false;
    if (m_localStationId != m_jointSessionHostStationId)
        return false;

    const IMatchmakeSession* bufferSession =
        m_matchmakeSession[m_currentSessionIndex == 0 ? 1 : 0];
    if (bufferSession == 0)
        return false;
    return bufferSession->IsMatchmakeOwner();
}

transport::Station::IdentificationToken* Session::GetMatchmakeAnalysisToken()
{
    return &m_matchmakeAnalysisToken;
}

bool Session::SessionJoiningApprovalCallback(
    const transport::Station::IdentificationToken* identificationToken)
{
    if (g_piaSession->m_joiningApprovalCallback == 0)
        return true;
    return g_piaSession->m_joiningApprovalCallback(identificationToken);
}

void Session::SetSyncClockPulseInterval(int interval)
{
    if (g_piaMesh != 0)
        g_piaMesh->SetSyncClockPulseInterval(interval);
}

}}}
