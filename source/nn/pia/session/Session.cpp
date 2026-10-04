#include "nn/pia/session/Session.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/JoinSessionJob.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/RandomMatchmakeJob.hpp"
#include "nn/pia/session/SessionStatusCheckJob.hpp"
#include "nn/pia/session/SessionSupport.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"
#include "nn/pia/session/UpdateSessionSettingJob.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
enum
{
    cStatusMatchedCurrent = 1,
    cStatusMatchedBuffer = 3,
    cMatchmakeStatusUnknown3 = 3,
    cMatchmakeStatusUnknown4 = 4,
    cResultOwnerChanged = 0x6c59,
    cTraceMaskStationTable = 0x4000000,
    cRankingUnknownStation = 0xfd,
    cRankingUnknownSession = 0xfc,
    cResultStationIdConvertDisabled = 0x4c11
};

unsigned char s_checkStartupSettingRange = 1;

const unsigned char s_statusByDisconnectStatus[7] = { 0, 0, 4, 5, 6, 7, 8 };
const unsigned char s_statusByMatchmakeStatus[5] = { 0, 0, 1, 2, 3 };
const unsigned char s_joinRandomStateByJoinMeshState[4] = { 4, 5, 6, 7 };
const unsigned char s_joinSessionStateByJoinMeshState[4] = { 4, 5, 6, 7 };
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

unsigned int Session::GetJoinRandomSessionStateForDebug() const
{
    unsigned int state = m_randomMatchmakeJob->GetJoinSessionStateForDebug();
    if (state == 4)
    {
        const unsigned int joinMeshState = GetJoinMeshState();
        if (joinMeshState < 4)
            state = s_joinRandomStateByJoinMeshState[joinMeshState];
        else
            state = 9;
    }
    return state;
}

unsigned int Session::GetJoinSessionStateForDebug() const
{
    unsigned int state = m_joinSessionJob->GetJoinSessionStateForDebug();
    if (state == 4)
    {
        const unsigned int joinMeshState = GetJoinMeshState();
        if (joinMeshState > 3)
            state = 9;
        else
            state = s_joinSessionStateByJoinMeshState[joinMeshState];
    }
    return state;
}

bool Session::IsProcessingJoin() const
{
    bool processing = false;
    if (m_randomMatchmakeJob != 0)
        processing = m_randomMatchmakeJob->IsRunning();
    if (m_joinSessionJob != 0)
        processing |= m_joinSessionJob->IsRunning();
    return processing;
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
    m_disconnectStatus = cConnectionStatusDisconnectedSession;
    m_hostStationId = 0;
    m_jointSessionHostStationId = 0;
    CheckStatusAtChangeMeshState();
    if (m_sessionDisconnectReason == 0)
        m_sessionDisconnectReason = 6;
}

unsigned short Session::GetParticipantNum() const
{
    return m_participantNum;
}

bool Session::IsHostMigrationEnabled() const
{
    if (m_meshLayerController != 0 && m_meshLayerController->IsStartedUp())
        return m_meshLayerController->IsHostMigrationEnabled();
    return false;
}

unsigned int Session::GetMatchmakeSessionStationNum() const
{
    if (m_meshLayerController != 0)
        return m_meshLayerController->GetMatchmakeSessionTotalStations();
    return 0;
}

Session::DisconnectionEventListener Session::GetDisconnectionEventListener()
{
    return DisconnectionEventListenerForSession;
}

MeshEventListener* Session::GetMeshEventListenerPtr()
{
    return m_meshEventListener;
}

bool DisconnectionEventListenerForSession(void* event)
{
    return g_piaSession->m_stationIdStatusTable->DisconnectionEventListenerForJointSession(event);
}

unsigned int Session::GetSessionIdCallback()
{
    Session* session = g_piaSession;
    unsigned int sessionId = 0;
    if (session != 0)
        sessionId = session->m_sessionId[session->m_currentSessionIndex];
    return sessionId;
}

unsigned char Session::CheckHostCandidateRankingCallback(StationIndex index, bool checkHost)
{
    if (!g_piaSession->IsHostMigrationEnabled())
        return 0;
    JointSessionJob* jointSessionJob = g_piaSession->m_jointSessionJob;
    if (jointSessionJob == 0)
        return 0;
    if (g_piaSession->m_matchmakeStatus != cMatchmakeStatusUnknown4 &&
        (g_piaSession->m_matchmakeStatus != cMatchmakeStatusUnknown3 || jointSessionJob->m_fielda3 == 0))
        return 0;
    transport::g_piaTransport->m_stationIdTable->Trace(cTraceMaskStationTable);
    g_piaSession->m_stationIdStatusTable->Trace(cTraceMaskStationTable);
    transport::StationIdTable::Entry entry;
    if (transport::g_piaTransport->m_stationIdTable->FindByStationIndex(&entry, index) != 0)
        return cRankingUnknownStation;
    unsigned int entrySessionId = 0;
    if (!g_piaSession->m_stationIdStatusTable->GetSessionId(entry.stationId, &entrySessionId))
        return cRankingUnknownSession;
    if (entrySessionId == g_piaSession->m_field110)
        return 0;
    const unsigned char rank = g_piaSession->m_stationIdStatusTable->GetSessionIdRank(entrySessionId);
    bool isSessionHost = false;
    if (checkHost)
        g_piaSession->m_stationIdStatusTable->GetSessionHost(entry.stationId, &isSessionHost);
    if (entry.stationId != g_piaSession->m_hostStationId && !isSessionHost)
        return static_cast<unsigned char>(rank * 4 + 1);
    return static_cast<unsigned char>((rank & 0x3f) << 2);
}

unsigned short Session::GetValidStationIdNum() const
{
    return static_cast<unsigned short>(m_validStationIdList.GetNodeNum());
}

unsigned short Session::GetStationNum() const
{
    return GetValidStationIdNum();
}

unsigned long long Session::GetJointSessionHostStationId() const
{
    if (m_jointSessionJob == 0)
        return 0;
    return m_jointSessionHostStationId;
}

void Session::NotifyOwnershipChange(unsigned int sessionId, unsigned long long stationId)
{
    if (m_joinSessionJob != 0 && m_joinSessionJob->IsRunning())
    {
        m_joinSessionJob->NoticeOwnerChangeEvent(sessionId, stationId);
        return;
    }
    if (m_randomMatchmakeJob != 0 && m_randomMatchmakeJob->IsRunning())
    {
        m_randomMatchmakeJob->NoticeOwnerChangeEvent(sessionId, stationId);
        return;
    }
    if (m_leaveSessionJob != 0 && m_leaveSessionJob->IsRunning())
        return;
    if (m_disconnectStatus - cConnectionStatusDisconnectedSession < 5)
        return;

    const unsigned char bufferIndex = m_currentSessionIndex == 0 ? 1 : 0;
    if (sessionId != m_sessionId[m_currentSessionIndex] && sessionId != m_sessionId[bufferIndex])
        return;

    transport::StationIdTable* stationIdTable = transport::g_piaTransport->m_stationIdTable;
    transport::StationIdTable::Entry entry;
    if (stationIdTable->FindByUniqueId(&entry, stationId) != 0)
    {
        transport::IdentificationInfo info;
        if (transport::IdentificationInfoTable::GetInstance()->GetIdentificationInfo(stationId, &info) == 0)
            return;
        if (!g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
        {
            if (m_sessionId[m_currentSessionIndex] == sessionId)
            {
                if (m_jointSessionJob == 0 || m_jointSessionJob->m_processType == cProcessTypeNone)
                {
                    ChangeStatusToDisconnectedSession();
                    return;
                }
                m_jointSessionJob->m_fielda2 = 1;
                m_jointSessionJob->m_fielda4 = cResultOwnerChanged;
            }
        }
        else if (g_piaMesh->GetProcessHostMigrationJobPtr()->m_unknownA5)
        {
            g_piaMesh->GetProcessHostMigrationJobPtr()->CheckWhetherNewHostIsUnknown(stationId);
        }
    }

    if (m_sessionId[m_currentSessionIndex] != sessionId)
    {
        if (m_matchmakeStatus != cMatchmakeStatusUnknown4)
        {
            if (m_matchmakeStatus != cMatchmakeStatusUnknown3)
                return;
            if (m_jointSessionHostStationId == stationId || m_jointSessionHostStationId != m_localStationId)
            {
                m_jointSessionJob->NoticeOwnerChangeEventImpl(sessionId, stationId);
                return;
            }
            if (m_jointSessionJob == 0 || m_jointSessionJob->m_processType == cProcessTypeNone)
            {
                ChangeStatusToDisconnectedSession();
                return;
            }
            if (m_jointSessionJob->m_processType != cProcessTypeDestroyJointSession &&
                m_jointSessionJob->m_processType != cProcessTypeLeaveJointSession)
            {
                m_jointSessionJob->m_fielda2 = 1;
                m_jointSessionJob->m_fielda4 = cResultOwnerChanged;
                m_jointSessionJob->NoticeOwnerChangeEventImpl(sessionId, stationId);
                return;
            }
            if (stationIdTable->FindByUniqueId(&entry, stationId) == 0)
            {
                unsigned int entrySessionId = 0;
                m_stationIdStatusTable->GetSessionId(entry.stationId, &entrySessionId);
                if (entrySessionId == m_sessionId[m_currentSessionIndex])
                {
                    m_jointSessionJob->m_fielda2 = 1;
                    m_jointSessionJob->m_fielda4 = cResultOwnerChanged;
                    m_jointSessionJob->NoticeOwnerChangeEventImpl(sessionId, stationId);
                    return;
                }
            }
            m_jointSessionJob->NoticeOwnerChangeEventImpl(sessionId, stationId);
            return;
        }
        bool isJointSessionHost;
        if (m_jointSessionHostStationId == stationId)
        {
            if (stationIdTable->FindByUniqueId(&entry, stationId) != 0)
                return;
            unsigned int entrySessionId = 0;
            if (!m_stationIdStatusTable->GetSessionId(entry.stationId, &entrySessionId))
                entrySessionId = 0;
            m_field110 = entrySessionId;
            isJointSessionHost = IsJointSessionHost();
        }
        else
        {
            if (m_jointSessionHostStationId == m_localStationId)
            {
                if (m_jointSessionJob == 0 || m_jointSessionJob->m_processType == cProcessTypeNone)
                {
                    ChangeStatusToDisconnectedSession();
                    return;
                }
                m_jointSessionJob->m_fielda2 = 1;
                m_jointSessionJob->m_fielda4 = cResultOwnerChanged;
            }
            isJointSessionHost = IsJointSessionHost();
        }
        if (!isJointSessionHost)
            return;
        CallSessionEventCallback(cEventTypeUnknown04, GetJointSessionHostStationId());
    }
    else if (m_matchmakeStatus == cMatchmakeStatusUnknown4)
    {
        if (m_field110 != sessionId)
        {
            if (m_hostStationId != stationId)
            {
                if (stationIdTable->FindByStationId(&entry, m_hostStationId) != 0)
                {
                    ProcessHostMigrationJob* migrationJob = g_piaMesh->GetProcessHostMigrationJobPtr();
                    if (migrationJob->m_started)
                    {
                        migrationJob = g_piaMesh->GetProcessHostMigrationJobPtr();
                        if (!migrationJob->m_started)
                            return;
                        migrationJob = g_piaMesh->GetProcessHostMigrationJobPtr();
                        if (!migrationJob->m_unknownB8)
                            return;
                    }
                    m_hostStationId = stationId;
                    if (m_localStationId == stationId)
                    {
                        m_matchmakeSession[m_currentSessionIndex]->UpdateMatchmakeSessionHost(sessionId);
                        stationId = m_hostStationId;
                    }
                    CallSessionEventCallback(cEventTypeUnknown02, stationId);
                    return;
                }
                if (m_localStationId != m_hostStationId)
                    return;
            }
            ChangeStatusToDisconnectedSession();
            return;
        }
        const bool isJointSessionHost = IsJointSessionHost();
        if (isJointSessionHost)
            CallSessionEventCallback(cEventTypeUnknown04, GetJointSessionHostStationId());
        if (!IsHost())
        {
            if (!isJointSessionHost)
                return;
        }
        else
        {
            CallSessionEventCallback(cEventTypeUnknown02, m_hostStationId);
        }
    }
    else
    {
        if (m_matchmakeStatus != cMatchmakeStatusMatched)
        {
            if (m_matchmakeStatus != cMatchmakeStatusUnknown3)
                return;
            if (m_jointSessionJob == 0 || !m_jointSessionJob->IsRunning())
                return;
            if (m_hostStationId != stationId && m_localStationId == m_hostStationId)
            {
                m_jointSessionJob->m_fielda2 = 1;
                m_jointSessionJob->m_fielda4 = cResultOwnerChanged;
                m_jointSessionJob->NoticeOwnerChangeEventImpl(sessionId, stationId);
                return;
            }
            if (m_jointSessionJob->m_processType == cProcessTypeDestroyJointSession)
            {
                unsigned int entrySessionId;
                bool needsNotice;
                if (!m_stationIdStatusTable->GetSessionId(m_jointSessionHostStationId, &entrySessionId))
                {
                    entrySessionId = 0;
                    needsNotice = sessionId != 0;
                }
                else
                {
                    needsNotice = entrySessionId != sessionId;
                }
                if (needsNotice)
                {
                    if (stationIdTable->FindByStationId(&entry, m_hostStationId) != 0 ||
                        !m_stationIdStatusTable->CheckConnectStatus(m_hostStationId))
                    {
                        if (m_localStationId == stationId)
                            m_matchmakeSession[m_currentSessionIndex]->UpdateMatchmakeSessionHost(sessionId);
                        m_jointSessionJob->NoticeHostChangeEventImpl(stationId);
                    }
                }
            }
            m_jointSessionJob->NoticeOwnerChangeEventImpl(sessionId, stationId);
            return;
        }
        if (!IsHost())
        {
            if (m_hostStationId != m_localStationId)
                return;
            if (m_hostStationId == stationId)
                return;
            ChangeStatusToDisconnectedSession();
            return;
        }
        CallSessionEventCallback(cEventTypeUnknown02, m_hostStationId);
    }
    if (m_postponeNotice)
    {
        m_postponeNotice = false;
        m_stationIdStatusTable->ClearDisconnectedStationId();
        m_stationIdStatusTable->CallJoinEventNotYetNotified();
    }
}

void Session::CancelJoinProcessByFatalError()
{
    if (m_randomMatchmakeJob != 0 && m_randomMatchmakeJob->IsRunning())
        m_randomMatchmakeJob->m_unknown57 = true;
    if (m_joinSessionJob != 0 && m_joinSessionJob->IsRunning())
        m_joinSessionJob->m_sessionInvalidated = true;
}

void Session::invalidateStationId(unsigned long long stationId)
{
    common::ListNode* sentinel = m_validStationIdList.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        if (reinterpret_cast<StationIdEntry*>(node)->m_stationId == stationId)
        {
            m_validStationIdList.EraseNode(node);
            m_validStationIdFreeList.InsertAfterNode(
                m_validStationIdFreeList.Sentinel(),
                reinterpret_cast<common::ListNode*>(
                    reinterpret_cast<char*>(node) + m_stationIdNodeOffset));
            break;
        }
    }
    UpdateSessionStationInfo();
}

void Session::UpdateSessionStationInfo()
{
    const unsigned int status = GetStatus();
    if ((status != cStatusMatchedCurrent && status != cStatusMatchedBuffer) ||
        m_leaveSessionJob->IsRunning())
        return;
    common::ListNode* sentinel = m_validStationIdList.Sentinel();
    unsigned short playerNum = 0;
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        transport::Station* station = transport::g_piaStationManager->GetStation(
            reinterpret_cast<StationIdEntry*>(node)->m_stationId);
        unsigned char count;
        if (station != 0 && station->GetPlayerCount(&count) == 0)
            playerNum += count;
    }
    m_playerNum = playerNum;
    unsigned short participantNum = 0;
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        transport::Station* station = transport::g_piaStationManager->GetStation(
            reinterpret_cast<StationIdEntry*>(node)->m_stationId);
        unsigned char count;
        if (station != 0 && station->GetParticipantCount(&count) == 0)
            participantNum += count;
    }
    m_participantNum = participantNum;
    if (GetStatus() == cStatusMatchedCurrent)
        m_matchmakeSession[m_currentSessionIndex]->UpdateStationInfoList();
    else if (GetStatus() == cStatusMatchedBuffer)
        m_matchmakeSession[m_currentSessionIndex == 0 ? 1 : 0]->UpdateStationInfoList();
}

void Session::ChangeStationIdConvertEnableFlag(bool enable)
{
    if (!enable)
        transport::g_piaTransport->SetTransportStatus(cResultStationIdConvertDisabled);
    else
        transport::g_piaTransport->SetTransportStatus(0);
}

unsigned int Session::GetJointSessionStateForDebug() const
{
    if (m_jointSessionJob != 0)
        return m_jointSessionJob->GetJointSessionStateForDebug();
    return 0;
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
    m_disconnectStatus = cConnectionStatusDisconnectedNetwork;
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
