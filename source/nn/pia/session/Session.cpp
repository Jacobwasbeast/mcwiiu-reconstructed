#include "nn/pia/session/Session.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/JoinSessionJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/SessionStatusCheckJob.hpp"
#include "nn/pia/session/SessionSupport.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"
#include "nn/pia/session/UpdateSessionSettingJob.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
unsigned char s_checkStartupSettingRange = 1;

const unsigned char s_statusByDisconnectStatus[7] = { 0, 0, 4, 5, 6, 7, 8 };
const unsigned char s_statusByMatchmakeStatus[5] = { 0, 0, 1, 2, 3 };
}

Session* g_piaSession = 0;
unsigned char g_piaMatchmakeAnalysisEnabled = 0;

Result Session::Startup(const StartupSetting& setting)
{
    if (IsDuringSetup() != 0)
        return static_cast<Result>(0x10c08);
    if (m_startupState != 0)
        return static_cast<Result>(0x10c08);
    if (s_checkStartupSettingRange != 0 && setting.m_field00 - 1000 >= 29001)
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

bool Session::IsValidStationId(unsigned long long stationId) const
{
    return m_stationIdStatusTable->CheckConnectStatus(stationId);
}

bool Session::IsHost() const
{
    if (m_localStationId == 0)
        return false;
    if (m_localStationId != m_hostStationId)
        return false;

    const IMatchmakeSession* matchmakeSession = m_matchmakeSession[m_currentSessionIndex];
    if (matchmakeSession != 0 && matchmakeSession->IsMatchmakeOwner())
        return true;
    return false;
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
    if (bufferSession != 0 && bufferSession->IsMatchmakeOwner())
        return true;
    return false;
}

unsigned int Session::GetStatus() const
{
    if (m_disconnectStatus > 1 && m_disconnectStatus < 7)
        return s_statusByDisconnectStatus[m_disconnectStatus];
    if (m_matchmakeStatus > 1 && m_matchmakeStatus < 5)
        return s_statusByMatchmakeStatus[m_matchmakeStatus];
    return 0;
}

unsigned int Session::GetJoinMeshState() const
{
    return g_piaMesh->GetJoinMeshState();
}

unsigned int Session::GetSessionOpenStatus() const
{
    if (!IsHost())
        return cSessionOpenStatusClosed;
    return m_sessionOpenStatus[m_currentSessionIndex];
}

void Session::SetCurrentSessionOpenStatus(SessionOpenStatus status)
{
    m_sessionOpenStatus[m_currentSessionIndex] = static_cast<unsigned int>(status);
}

void Session::SetBufferSessionOpenStatus(SessionOpenStatus status)
{
    const unsigned int bufferIndex = m_currentSessionIndex == 0 ? 1 : 0;
    m_sessionOpenStatus[bufferIndex] = static_cast<unsigned int>(status);
}

bool Session::UpdateStationIdToBecomeHost()
{
    if (m_sessionId[0] == 0)
        return false;
    g_piaMesh->GetLocalStationIndex();
    const unsigned long long principalId =
        transport::g_stationConnectionInfoTable->GetPrincipalIdByStation(
            transport::g_piaStationManager->m_localStation);
    if (principalId == 0)
        return false;
    m_localStationId = principalId;
    m_hostStationId = principalId;
    if (m_matchmakeStatus == cMatchmakeStatusMatched)
        return true;
    return false;
}

void Session::CheckStatusAtChangeMeshState()
{
    const unsigned int result = m_sessionStatusCheckJob->CheckSessionStatus4JointSession();
    if ((result >> 24) != 5)
        m_sessionStatusCheckJob->Reset(false);
}

void Session::ChangeStatusToDisconnectedSession()
{
    m_disconnectStatus = 2;
    m_hostStationId = 0;
    m_jointSessionHostStationId = 0;
    CheckStatusAtChangeMeshState();
    if (m_sessionDisconnectReason == 0)
        m_sessionDisconnectReason = 6;
}

bool Session::UpdateStationIdToBecomeClient()
{
    g_piaMesh->GetLocalStationIndex();
    const unsigned long long localPrincipalId =
        transport::g_stationConnectionInfoTable->GetPrincipalIdByStation(
            transport::g_piaStationManager->m_localStation);
    if (localPrincipalId == 0)
        return false;

    m_localStationId = localPrincipalId;
    const unsigned long long hostPrincipalId =
        transport::g_stationConnectionInfoTable->GetPrincipalIdByStation(
            transport::g_piaStationManager->GetStation(g_piaMesh->GetHostStationIndex()));
    if (hostPrincipalId == 0)
        return false;

    m_field110 = 0;
    if (m_matchmakeStatus == 4)
        return false;
    m_hostStationId = hostPrincipalId;
    return true;
}

void Session::ChangeStatusToDisconnectedNetwork()
{
    m_disconnectStatus = 3;
    m_hostStationId = 0;
    m_jointSessionHostStationId = 0;
    CheckStatusAtChangeMeshState();
    if (m_sessionDisconnectReason == 0)
        m_sessionDisconnectReason = 8;
}

void Session::CallSessionEventCallback(EventType eventType, unsigned long long stationId)
{
    if ((eventType != cEventTypeUnknown03 && eventType != cEventTypeUnknown02) ||
        (!IsProcessingJoin() && !m_leaveSessionJob->IsRunning() &&
         m_disconnectStatus - 2 > 4))
    {
        if (m_sessionEventCallback != 0)
            m_sessionEventCallback(eventType, stationId, m_sessionEventCallbackParam);
    }
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

void Session::ResetInternalAsyncState()
{
    if (m_asyncProcessingName != 0 && m_callContext.m_state != common::CallContext::cStateInProgress)
    {
        m_callContext.Reset();
        m_asyncProcessingName = cAsyncProcessingNameNone;
    }
}

Result Session::JoinSessionAsync(const JoinSessionSetting* setting)
{
    if (setting == 0)
        return static_cast<Result>(0x10c07);
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    if (m_startupState != 1)
        return static_cast<Result>(0x10c08);
    if (m_matchmakeStatus != cMatchmakeStatusNone)
        return static_cast<Result>(0x10c08);

    ResetInternalAsyncState();
    Result result = m_joinSessionJob->Startup(&m_callContext, setting);
    if (result == 0)
    {
        m_joinSessionJob->Ready(false);
        m_asyncProcessingName = cAsyncProcessingNameJoinSession;
    }
    return result;
}

Result Session::UpdateSessionSettingAsync(const UpdateSessionSetting* setting)
{
    IMatchmakeSession* matchmakeSession = m_matchmakeSession[m_currentSessionIndex];
    unsigned int sessionId = m_sessionId[m_currentSessionIndex];
    Result result = UpdateSessionSettingAsyncCore(sessionId, matchmakeSession, setting, false);
    if (result == 0)
        m_asyncProcessingName = cAsyncProcessingNameUpdateSessionSetting;
    return result;
}

Result Session::UpdateSessionSettingAsyncCore(unsigned int sessionId,
                                              IMatchmakeSession* matchmakeSession,
                                              const UpdateSessionSetting* setting,
                                              bool isJointSession)
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    if (m_startupState != 1)
        return static_cast<Result>(0x10c08);
    if (m_updateSessionSettingJob->IsRunning())
        return static_cast<Result>(0x10c08);

    ResetInternalAsyncState();
    if (m_updateSessionSettingJob == 0)
        return static_cast<Result>(0x10c08);

    Result result = m_updateSessionSettingJob->Startup(
        &m_callContext, sessionId, matchmakeSession, setting, isJointSession);
    if (result == 0)
        m_updateSessionSettingJob->Ready(false);
    return result;
}

}}}
