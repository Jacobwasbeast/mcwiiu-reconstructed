#include "nn/pia/session/JointSessionJob.hpp"

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/CreateSessionSetting.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/JoinRandomSessionSetting.hpp"
#include "nn/pia/session/JoinSessionSetting.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"
#include "nn/pia/transport/BandwidthCheckProtocol.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "platform/wiiu/pead.hpp"

#include <string.h>

namespace nn { namespace pia { namespace session {

namespace
{
enum
{
    cDisconnectStatusDisconnectedSession = 2,
    cDisconnectStatusDisconnectedNetwork = 3,
    cDisconnectStatus4 = 4,
    cDisconnectStatus5 = 5,
    cDisconnectStatus6 = 6,
    cDisconnectReasonSession = 6,
    cDisconnectReasonNetwork = 8
};

enum
{
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08
};

enum
{
    cMatchmakeStatusMatched = 2,
    cMatchmakeStatusJointSessionProcessing = 3,
    cMatchmakeStatusJointSessionProcessed = 4
};

enum
{
    cProcessTypeFirstJointSession = 6,
    cProcessTypeNum = 11,
    cStationNumMax = 16,
    cMonitoringByteUnset = 0xff,
    cMonitoringByteLimit = 0xfe
};

enum
{
    cEventTypeStartCreateJointSession = 11,
    cEventTypeEndCreateJointSession = 12,
    cEventTypeStartJoinJointSession = 13,
    cEventTypeEndJoinJointSession = 14,
    cEventTypeStartJoinRandomJointSession = 15,
    cEventTypeEndJoinRandomJointSession = 16,
    cEventTypeStartDestroyJointSession = 17,
    cEventTypeEndDestroyJointSession = 18,
    cEventTypeStartLeaveJointSession = 19,
    cEventTypeEndLeaveJointSession = 20
};

const unsigned long long cTraceJointSessionLevel = 0x0000000100000000ULL;

const unsigned char s_startEventTypeByProcessType[5] = {
    cEventTypeStartCreateJointSession, cEventTypeStartJoinJointSession,
    cEventTypeStartJoinRandomJointSession, cEventTypeStartDestroyJointSession,
    cEventTypeStartLeaveJointSession
};

const unsigned char s_endEventTypeByProcessType[5] = {
    cEventTypeEndCreateJointSession, cEventTypeEndJoinJointSession,
    cEventTypeEndJoinRandomJointSession, cEventTypeEndDestroyJointSession,
    cEventTypeEndLeaveJointSession
};

inline unsigned int getBufferSessionIndex()
{
    return g_piaSession->m_currentSessionIndex == 0 ? 1 : 0;
}

inline bool containsStationId(const common::ListBase& list, unsigned long long stationId)
{
    const common::ListNode* sentinel = list.Sentinel();
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        if (static_cast<const JointSessionJob::StationIdNode*>(node)->m_stationId == stationId)
            return true;
    }
    return false;
}

inline const transport::StationIdTable::Entry* entryOf(const common::ListNode* node)
{
    return &reinterpret_cast<const transport::StationIdTable::Element*>(node)->entry;
}

inline const Session::StationIdEntry* validStationIdOf(const common::ListNode* node)
{
    return reinterpret_cast<const Session::StationIdEntry*>(node);
}

inline int calcElapsedMsec(const common::Time& startTime)
{
    return static_cast<int>(
        static_cast<long long>(common::g_piaJobScheduler->m_dispatchTime.m_tick - startTime.m_tick) /
        common::TimeSpan::GetTicksPerMSec().m_tick);
}
}

JointSessionJob::JointSessionJob()
    : common::StepSequenceJob(), m_callerContext(0), m_callContext(),
      m_processType(Session::cProcessTypeNone), m_field5c(0), m_companionStationIdList(),
      m_companionStationIdFreeList(), m_stationIdNodeOffset(0),
      m_stationIdNodes(0), m_stationIdNodeNum(0), m_field98(0), m_field9c(0),
      m_field9d(0), m_leaveStationIdAccepted(false), m_fielda0(0),
      m_fielda2(0), m_fielda3(0), m_fielda4(0), m_fieldb0(0), m_fieldb4(0),
      m_startTime(), m_fieldc0(0), m_fieldc4(0)
{
    JointSessionJob::InitializeImpl();
}

void JointSessionJob::InitializeImpl()
{
    const unsigned int stationIdNodeNum = transport::g_piaTransport->m_stationNum;
    const unsigned int storageSize = stationIdNodeNum * sizeof(StationIdNode);
    pead::Heap* heap = common::HeapManager::GetHeap();
    StationIdNode* storage =
        static_cast<StationIdNode*>(pead::allocateFromSeadHeap(storageSize, heap));
    if (storage == 0)
    {
        m_stationIdNodeStorage = 0;
    }
    else
    {
        memset(storage, 0, storageSize);
        m_stationIdNodeStorage = storage;
    }

    if (stationIdNodeNum == 0 || storage == 0)
        return;

    m_stationIdNodes = storage;
    m_stationIdNodeNum = stationIdNodeNum;
    for (unsigned int index = 0; index < m_stationIdNodeNum; ++index)
    {
        StationIdNode* node = m_stationIdNodes + index;
        node->prev = 0;
        node->next = 0;
        m_companionStationIdFreeList.InsertBeforeNode(
            m_companionStationIdFreeList.Sentinel(),
            reinterpret_cast<common::ListNode*>(
                reinterpret_cast<char*>(node) + m_stationIdNodeOffset));
    }
}

void JointSessionJob::FinalizeImpl()
{
    if (m_stationIdNodeStorage == 0)
        return;

    if (m_stationIdNodes != 0)
    {
        common::ListNode* sentinel = m_companionStationIdList.Sentinel();
        common::ListNode* node = sentinel->next;
        while (node != sentinel)
        {
            common::ListNode* next = node->next;
            node->prev = 0;
            node->next = 0;
            m_companionStationIdFreeList.InsertAfterNode(
                m_companionStationIdFreeList.Sentinel(),
                reinterpret_cast<common::ListNode*>(
                    reinterpret_cast<char*>(node) + m_stationIdNodeOffset));
            node = next;
        }
        m_companionStationIdList.Init();
    }

    if (m_stationIdNodeStorage != 0)
        pead::DeleteArray(m_stationIdNodeStorage);
    m_stationIdNodeStorage = 0;
}

JointSessionJob::~JointSessionJob()
{
    JointSessionJob::FinalizeImpl();
}

Result JointSessionJob::StartupJoinRandomJointSession(common::CallContext* callerContext,
                                                      const JoinRandomSessionSetting* setting)
{
    if (callerContext == 0 || setting == 0)
        return cResultInvalidArgument;
    if (m_processType != Session::cProcessTypeNone)
        return cResultInvalidState;

    g_piaSession->m_matchmakeSession[getBufferSessionIndex()]->Cleanup();
    const Result result = StartupJoinRandomJointSessionImpl(setting);
    if (result != 0)
        return result;

    m_field9c = 1;
    m_leaveStationId = g_piaSession->m_localStationId;
    m_leaveStationIdAccepted = false;
    m_processType = Session::cProcessTypeJoinRandomJointSession;
    m_sessionDisconnectDetected = false;
    m_fielda2 = 0;
    m_fielda4 = 0;
    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_startTime = common::g_piaJobScheduler->m_dispatchTime;
    m_fieldc8 = 0;
    Reset(true);
    common::g_sessionBeginMonitoringContent.m_metric107 =
        setting->GetCreateSessionSetting()->GetMaxParticipantNum();
    return 0;
}

Result JointSessionJob::StartupCreateJointSession(common::CallContext* callerContext,
                                                  const CreateSessionSetting* setting)
{
    if (callerContext == 0 || setting == 0)
        return cResultInvalidArgument;
    if (m_processType != Session::cProcessTypeNone)
        return cResultInvalidState;

    g_piaSession->m_matchmakeSession[getBufferSessionIndex()]->Cleanup();
    const Result result = StartupCreateJointSessionImpl(setting);
    if (result != 0)
        return result;

    m_field9c = 1;
    m_leaveStationId = g_piaSession->m_localStationId;
    m_fielda4 = 0;
    m_processType = Session::cProcessTypeCreateJointSession;
    m_leaveStationIdAccepted = false;
    m_fielda2 = 0;
    m_callerContext = callerContext;
    m_sessionDisconnectDetected = false;
    callerContext->InitiateCall();
    m_startTime = common::g_piaJobScheduler->m_dispatchTime;
    m_fieldc8 = 0;
    Reset(true);
    common::g_sessionBeginMonitoringContent.m_metric107 = setting->GetMaxParticipantNum();
    return 0;
}

Result JointSessionJob::StartupJoinJointSession(common::CallContext* callerContext,
                                                const JoinSessionSetting* setting)
{
    if (callerContext == 0 || setting == 0)
        return cResultInvalidArgument;
    if (setting->GetSessionId() == 0)
        return cResultInvalidArgument;
    if (setting->GetMaxParticipantNum() == 0)
        return cResultInvalidArgument;
    if (setting->GetMaxParticipantNum() > transport::g_piaTransport->m_stationNum)
        return cResultInvalidArgument;
    if (m_processType != Session::cProcessTypeNone)
        return cResultInvalidState;

    m_field98 = setting->GetSessionId();
    g_piaSession->m_fieldc2[getBufferSessionIndex()] =
        static_cast<unsigned char>(setting->GetMaxParticipantNum());
    g_piaSession->m_matchmakeSession[getBufferSessionIndex()]->Cleanup();
    const Result result = StartupJoinJointSessionImpl(setting);
    if (result != 0)
        return result;

    m_field9c = 1;
    m_leaveStationId = g_piaSession->m_localStationId;
    m_leaveStationIdAccepted = false;
    m_processType = Session::cProcessTypeJoinJointSession;
    m_sessionDisconnectDetected = false;
    m_fielda2 = 0;
    m_fielda4 = 0;
    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_startTime = common::g_piaJobScheduler->m_dispatchTime;
    m_fieldc8 = 0;
    Reset(true);
    common::g_sessionBeginMonitoringContent.m_metric107 = setting->GetMaxParticipantNum();
    return 0;
}

Result JointSessionJob::StartupLeaveJointSession(common::CallContext* callerContext)
{
    if (callerContext == 0)
        return cResultInvalidArgument;
    if (m_processType != Session::cProcessTypeNone)
        return cResultInvalidState;

    const Result result = StartupLeaveJointSessionImpl();
    if (result != 0)
        return result;

    m_field9c = 1;
    m_leaveStationId = g_piaSession->m_localStationId;
    m_leaveStationIdAccepted = false;
    m_sessionDisconnectDetected = false;
    m_processType = Session::cProcessTypeLeaveJointSession;
    m_callerContext = callerContext;
    m_fielda4 = 0;
    m_fielda2 = 0;
    callerContext->InitiateCall();
    m_startTime = common::g_piaJobScheduler->m_dispatchTime;
    m_fieldc8 = 0;
    m_fielda3 = 1;
    Reset(true);
    return 0;
}

Result JointSessionJob::StartupDestroyJointSession(common::CallContext* callerContext)
{
    if (callerContext == 0)
        return cResultInvalidArgument;
    if (m_processType != Session::cProcessTypeNone)
        return cResultInvalidState;

    const Result result = StartupDestroyJointSessionImpl();
    if (result != 0)
        return result;

    m_field9c = 1;
    m_leaveStationId = g_piaSession->m_localStationId;
    m_leaveStationIdAccepted = false;
    m_sessionDisconnectDetected = false;
    m_processType = Session::cProcessTypeDestroyJointSession;
    m_callerContext = callerContext;
    m_fielda4 = 0;
    m_fielda2 = 0;
    callerContext->InitiateCall();
    m_startTime = common::g_piaJobScheduler->m_dispatchTime;
    m_fieldc8 = 0;
    m_fielda3 = 1;
    Reset(true);
    return 0;
}

Result JointSessionJob::StartupPassiveProcess(Session::ProcessType processType,
                                              SessionProtocol::MessageType messageType,
                                              const unsigned long long* stationIds,
                                              unsigned short stationIdNum,
                                              unsigned long long senderStationId)
{
    if (processType == Session::cProcessTypeNone || processType >= cProcessTypeNum)
        return cResultInvalidArgument;
    if (stationIds == 0 || stationIdNum == 0 || stationIdNum > m_stationIdNodeNum)
        return cResultInvalidArgument;
    if (senderStationId == 0)
        return cResultInvalidArgument;
    if (senderStationId == g_piaSession->m_localStationId)
        return cResultInvalidState;

    if (processType == Session::cProcessTypeDestroyJointSession ||
        processType == Session::cProcessTypeLeaveJointSession)
    {
        if (g_piaSession->m_matchmakeStatus != cMatchmakeStatusJointSessionProcessed)
            return cResultInvalidState;
        if (g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex] == 0)
            return cResultInvalidState;
        m_fielda3 = 1;
    }
    else
    {
        if (g_piaSession->m_matchmakeStatus != cMatchmakeStatusMatched)
            return cResultInvalidState;
        const unsigned int bufferIndex = getBufferSessionIndex();
        if (g_piaSession->m_sessionId[bufferIndex] != 0)
            return cResultInvalidState;
        g_piaSession->m_matchmakeSession[bufferIndex]->Cleanup();
        m_fielda3 = 0;
    }

    if (g_piaSession->m_callContext.m_state == common::CallContext::cStateInProgress)
        return cResultInvalidState;

    const Result result = StartupPassiveProcessImpl(processType, messageType, stationIds,
                                                    stationIdNum, senderStationId);
    if (result != 0)
        return result;

    m_field9c = 0;
    if (processType == Session::cProcessTypeDestroyJointSession &&
        messageType == SessionProtocol::cMessageTypeDestroyInvitation)
    {
        m_leaveStationIdAccepted = false;
        m_processType = processType;
        m_leaveStationId = 0;
        m_sessionDisconnectDetected = false;
        m_fielda2 = 0;
        m_fielda4 = 0;
        m_startTime = common::g_piaJobScheduler->m_dispatchTime;
        m_fieldc8 = 0;
        Reset(true);
    }
    else
    {
        m_leaveStationId = senderStationId;
        m_processType = processType;
        m_fielda4 = 0;
        m_sessionDisconnectDetected = false;
        m_leaveStationIdAccepted = false;
        m_fielda2 = 0;
        m_startTime = common::g_piaJobScheduler->m_dispatchTime;
        m_fieldc8 = 0;
        Reset(true);
    }
    return 0;
}

Result JointSessionJob::ReceiveNextSessionId(Session::ProcessType processType,
                                             unsigned int sessionId,
                                             unsigned long long matchmakeOwnerUniqueId,
                                             const unsigned char* applicationData,
                                             unsigned char applicationDataSize,
                                             unsigned long long senderStationId)
{
    if (m_processType == Session::cProcessTypeNone)
        return cResultInvalidState;
    if (m_processType != processType)
        return cResultInvalidState;
    if (senderStationId != m_leaveStationId)
        return cResultInvalidState;

    const Result result = ReceiveNextSessionIdImpl(processType, sessionId, applicationData,
                                                   applicationDataSize, senderStationId);
    if (result != 0)
        return result;

    if (matchmakeOwnerUniqueId != 0 && applicationData != 0 && applicationDataSize != 0)
    {
        IMatchmakeSession* matchmakeSession =
            g_piaSession->m_matchmakeSession[getBufferSessionIndex()];
        if (matchmakeSession->GetMatchmakeOwnerUniqueId() == 0)
            matchmakeSession->SetMatchmakeOwnerUniqueId(matchmakeOwnerUniqueId);
    }
    m_field98 = sessionId;
    return 0;
}

Result JointSessionJob::ReceiveLeaveInvitation(Session::ProcessType processType,
                                               const unsigned long long* stationIds,
                                               unsigned short stationIdNum,
                                               unsigned long long senderStationId)
{
    if (processType != Session::cProcessTypeDestroyJointSession)
        return cResultInvalidArgument;
    if (stationIds == 0 || stationIdNum == 0 || stationIdNum > m_stationIdNodeNum)
        return cResultInvalidArgument;
    if (senderStationId == 0)
        return cResultInvalidArgument;
    if (senderStationId == g_piaSession->m_localStationId)
        return cResultInvalidState;

    const Result result = ReceiveLeaveInvitationImpl(processType, stationIds, stationIdNum,
                                                     senderStationId);
    if (result != 0)
        return result;

    m_leaveStationId = senderStationId;
    m_processType = processType;
    m_leaveStationIdAccepted = false;
    m_field9c = 0;
    return 0;
}

Result JointSessionJob::ReceiveLeaveMeshInvitation(Session::ProcessType processType,
                                                   unsigned long long senderStationId)
{
    if (m_processType == Session::cProcessTypeNone)
        return cResultInvalidState;
    if (m_processType != processType)
        return cResultInvalidState;
    if (senderStationId != m_leaveStationId)
        return cResultInvalidState;

    const Result result = ReceiveLeaveMeshInvitationImpl(processType, senderStationId);
    if (result != 0)
        return result;
    return 0;
}

Result JointSessionJob::ReceiveCompletionResponse(Session::ProcessType processType,
                                                  unsigned long long senderStationId)
{
    if (m_processType == Session::cProcessTypeNone)
        return cResultInvalidState;
    if (m_processType != processType)
        return cResultInvalidState;

    const Result result = ReceiveCompletionResponseImpl(processType, senderStationId);
    if (result != 0)
        return result;
    return 0;
}

void JointSessionJob::Cleanup()
{
    CleanupImpl();

    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
    }
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();

    m_field5c = 0;
    m_fielda4 = 0;
    m_processType = Session::cProcessTypeNone;
    m_field9d = 0;
    m_field60 = 0;
    m_sessionDisconnectDetected = false;
    m_field9f = 0;
    m_leaveStationId = 0;
    m_fielda2 = 0;
    m_fielda3 = 0;
    m_field98 = 0;
    m_field9c = 0;
    m_fielda0 = 0;
    m_leaveStationIdAccepted = false;

    common::ListNode* sentinel = m_companionStationIdList.Sentinel();
    common::ListNode* node = sentinel->next;
    while (node != sentinel)
    {
        common::ListNode* next = node->next;
        node->prev = 0;
        node->next = 0;
        m_companionStationIdFreeList.InsertAfterNode(
            m_companionStationIdFreeList.Sentinel(),
            reinterpret_cast<common::ListNode*>(
                reinterpret_cast<char*>(node) + m_stationIdNodeOffset));
        node = next;
    }
    m_companionStationIdList.Init();
}

bool JointSessionJob::AcceptLeaveStationId(unsigned long long stationId)
{
    if (m_leaveStationId != stationId)
        return false;
    m_leaveStationIdAccepted = true;
    return true;
}

bool JointSessionJob::IsMigrateSessionToJointSession() const
{
    return m_processType >= Session::cProcessTypeCreateJointSession &&
           m_processType <= Session::cProcessTypeJoinRandomJointSession;
}

bool JointSessionJob::IsMigrateJointSessionToSession() const
{
    return m_processType == Session::cProcessTypeDestroyJointSession ||
           m_processType == Session::cProcessTypeLeaveJointSession;
}

void JointSessionJob::Trace(unsigned long long value) const
{
}

void JointSessionJob::NoticeStartingEventByProcessType()
{
    Session* session = g_piaSession;
    transport::StationIdTable* stationIdTable = transport::g_piaTransport->m_stationIdTable;
    const common::ListNode* sentinel = stationIdTable->m_entries.Sentinel();
    unsigned long long leaveStationIds[cStationNumMax];
    unsigned short leaveStationNum = 0;
    unsigned char companionStationNum = 0;

    if (IsMigrateJointSessionToSession() && m_field9c != 0)
    {
        for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
        {
            const transport::StationIdTable::Entry* entry = entryOf(node);
            unsigned int sessionId;
            if (session->m_stationIdStatusTable->GetSessionId(entry->stationId, &sessionId) &&
                sessionId == session->m_sessionId[session->m_currentSessionIndex])
            {
                ++companionStationNum;
            }
            else
            {
                leaveStationIds[leaveStationNum++] = entry->stationId;
            }
        }
    }
    else
    {
        for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
        {
            const transport::StationIdTable::Entry* entry = entryOf(node);
            if (containsStationId(m_companionStationIdList, entry->stationId))
                ++companionStationNum;
            else
                leaveStationIds[leaveStationNum++] = entry->stationId;
        }
    }

    for (unsigned short index = 0; index < leaveStationNum; ++index)
    {
        const unsigned long long stationId = leaveStationIds[index];
        if (session->m_stationIdStatusTable->CheckEventNotifiedStatus(stationId))
        {
            session->CallSessionEventCallback(Session::cEventTypeLeave, stationId);
            session->m_stationIdStatusTable->SetEventNotifiedStatus(stationId, false);
            session->m_stationIdStatusTable->SetConnect(stationId, false);
            session->m_stationIdStatusTable->SetPrepareStatus(stationId, false);
            session->invalidateStationId(stationId);
        }
    }

    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        const transport::StationIdTable::Entry* entry = entryOf(node);
        session->m_stationIdStatusTable->SetStationIndex(
            entry->stationId, static_cast<StationIndex>(entry->stationIndex));
    }

    unsigned int eventType = Session::cEventTypeLeave;
    if (m_processType > cProcessTypeFirstJointSession - 1 && m_processType < cProcessTypeNum)
        eventType = s_startEventTypeByProcessType[m_processType - cProcessTypeFirstJointSession];
    session->CallSessionEventCallback(static_cast<Session::EventType>(eventType), 0);
    transport::g_piaTransport->m_stationIdTable->Trace(cTraceJointSessionLevel);
    session->m_stationIdStatusTable->Trace(cTraceJointSessionLevel);
    session->m_matchmakeStatus = cMatchmakeStatusJointSessionProcessing;
    session->ChangeStationIdConvertEnableFlag(false);

    common::SessionBeginMonitoringContent& monitoring = common::g_sessionBeginMonitoringContent;
    if (monitoring.m_metric74 == cMonitoringByteUnset)
    {
        monitoring.m_metric74 = 1;
        monitoring.m_metric76 = session->m_sessionId[session->m_currentSessionIndex];
        monitoring.m_metric127 = static_cast<unsigned char>(m_processType);
    }
    else
    {
        if (monitoring.m_metric74 < cMonitoringByteLimit)
            ++monitoring.m_metric74;
        monitoring.m_metric76 = session->m_sessionId[session->m_currentSessionIndex];
        monitoring.m_metric127 = static_cast<unsigned char>(m_processType);
    }
    monitoring.m_metric78 = companionStationNum;
}

bool JointSessionJob::ClearProcessTypeAndNoticeEndingEvent()
{
    Session* session = g_piaSession;
    if (m_processType < cProcessTypeFirstJointSession || m_processType >= cProcessTypeNum)
        return false;

    const unsigned char eventType =
        s_endEventTypeByProcessType[m_processType - cProcessTypeFirstJointSession];
    transport::Station* hostStation =
        transport::g_piaStationManager->GetStation(g_piaMesh->GetHostStationIndex());
    if (hostStation == 0)
        return false;

    unsigned long long principalId;
    if (hostStation->GetPrincipalId(&principalId) != 0)
        return false;

    transport::StationIdTable* stationIdTable = transport::g_piaTransport->m_stationIdTable;
    transport::StationIdTable::Entry hostEntry;
    if (stationIdTable->FindByUniqueId(&hostEntry, principalId) != 0)
    {
        stationIdTable->Trace(cTraceJointSessionLevel);
        return false;
    }

    unsigned long long matchmakeOwnerUniqueId;
    if (!IsMigrateJointSessionToSession())
    {
        session->m_jointSessionHostStationId = hostEntry.stationId;
        unsigned int sessionId = 0;
        if (!session->m_stationIdStatusTable->GetSessionId(hostEntry.stationId, &sessionId))
            return false;
        if (sessionId == 0)
            return false;
        session->m_field110 = sessionId;
        if (session->m_matchmakeSession[getBufferSessionIndex()]->GetMatchmakeOwnerUniqueId() !=
            session->m_jointSessionHostStationId)
            return false;
        IMatchmakeSession* currentSession =
            session->m_matchmakeSession[session->m_currentSessionIndex];
        matchmakeOwnerUniqueId = currentSession->GetMatchmakeOwnerUniqueId();
        if (stationIdTable->FindByUniqueId(&hostEntry, matchmakeOwnerUniqueId) != 0)
            return false;
    }
    else
    {
        session->m_jointSessionHostStationId = 0;
        session->m_field110 = 0;
        matchmakeOwnerUniqueId =
            session->m_matchmakeSession[session->m_currentSessionIndex]->GetMatchmakeOwnerUniqueId();
        if (stationIdTable->FindByUniqueId(&hostEntry, matchmakeOwnerUniqueId) != 0)
            return false;
    }

    if (session->m_field110 == session->m_sessionId[session->m_currentSessionIndex] &&
        matchmakeOwnerUniqueId != session->m_jointSessionHostStationId)
        return false;

    const common::ListNode* validSentinel = session->m_validStationIdList.Sentinel();
    for (const common::ListNode* node = validSentinel->next; node != validSentinel;
         node = node->next)
    {
        const unsigned long long stationId = validStationIdOf(node)->m_stationId;
        if (session->m_stationIdStatusTable->CheckConnectStatus(stationId) &&
            session->m_stationIdStatusTable->CheckEventNotifiedStatus(stationId))
        {
            const transport::StationIdTable::Entry* entry =
                stationIdTable->FindByStationIdCore(stationId);
            if (entry == 0)
                return false;
            stationIdTable->SetJoinEventNotifiedByStationId(stationId, true);
            if (g_piaMesh->GetBitRateCheckMode())
            {
                g_piaMesh->GetBandwidthCheckProtocol()->SetBandwidthChecked(
                    static_cast<StationIndex>(entry->stationIndex));
            }
        }
    }

    session->ChangeStationIdConvertEnableFlag(true);
    if (m_processType >= Session::cProcessTypeCreateJointSession &&
        m_processType <= Session::cProcessTypeJoinRandomJointSession)
    {
        session->m_matchmakeStatus = cMatchmakeStatusJointSessionProcessed;
        session->m_stationIdStatusTable->ClearDisconnectedStationId();
        session->m_stationIdStatusTable->CallJoinEventNotYetNotified();
        stationIdTable->Trace(cTraceJointSessionLevel);
        session->m_stationIdStatusTable->Trace(cTraceJointSessionLevel);
    }
    else
    {
        session->m_matchmakeStatus = cMatchmakeStatusMatched;
        session->m_stationIdStatusTable->ClearDisconnectedStationId();
        session->m_stationIdStatusTable->CallJoinEventNotYetNotified();
        stationIdTable->Trace(cTraceJointSessionLevel);
        session->m_stationIdStatusTable->Trace(cTraceJointSessionLevel);
    }
    if (hostEntry.stationId != session->m_hostStationId)
    {
        session->m_hostStationId = hostEntry.stationId;
        session->CallSessionEventCallback(Session::cEventTypeUnknown02, hostEntry.stationId);
    }
    session->CallSessionEventCallback(static_cast<Session::EventType>(eventType), 0);

    common::SessionBeginMonitoringContent& monitoring = common::g_sessionBeginMonitoringContent;
    monitoring.m_metric77 = calcElapsedMsec(m_startTime);
    monitoring.m_metric79 = m_fieldc8;
    unsigned char companionStationNum = 0;
    const common::ListNode* sentinel = stationIdTable->m_entries.Sentinel();
    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        unsigned int sessionId;
        if (session->m_stationIdStatusTable->GetSessionId(entryOf(node)->stationId, &sessionId) &&
            sessionId == session->m_sessionId[session->m_currentSessionIndex])
        {
            ++companionStationNum;
        }
    }
    monitoring.m_metric158 = companionStationNum;
    m_processType = Session::cProcessTypeNone;
    return true;
}

void JointSessionJob::ClearProcessTypeAndNoticeFailureEvent()
{
    const unsigned int disconnectStatus = g_piaSession->m_disconnectStatus;
    if (disconnectStatus == cDisconnectStatusDisconnectedNetwork ||
        disconnectStatus == cDisconnectStatus4 || disconnectStatus == cDisconnectStatus5 ||
        disconnectStatus == cDisconnectStatus6)
    {
        g_piaSession->m_sessionDisconnectReason = cDisconnectReasonNetwork;
    }
    else
    {
        g_piaSession->m_disconnectStatus = cDisconnectStatusDisconnectedSession;
        g_piaSession->m_sessionDisconnectReason = cDisconnectReasonSession;
    }
    g_piaSession->m_stationIdStatusTable->ClearDisconnectedStationId();
    g_piaSession->CallSessionEventCallback(Session::cEventTypeUnknown15, 0);
    m_processType = Session::cProcessTypeNone;
    common::g_sessionBeginMonitoringContent.m_metric77 = calcElapsedMsec(m_startTime);
    common::g_sessionBeginMonitoringContent.m_metric79 = m_fieldc8;
}

void JointSessionJob::RemoveNotCompanionStationId()
{
    transport::StationIdTable* stationIdTable = transport::g_piaTransport->m_stationIdTable;
    StationIdStatusTable* stationIdStatusTable = g_piaSession->m_stationIdStatusTable;
    const common::ListNode* sentinel = stationIdTable->m_entries.Sentinel();
    unsigned long long removeStationIds[cStationNumMax];
    unsigned short removeStationNum = 0;

    for (const common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        const unsigned long long stationId = entryOf(node)->stationId;
        if (!containsStationId(m_companionStationIdList, stationId) &&
            !stationIdStatusTable->CheckEventNotifiedStatus(stationId))
        {
            removeStationIds[removeStationNum++] = stationId;
        }
    }

    for (unsigned short index = 0; index < removeStationNum; ++index)
    {
        stationIdTable->RemoveByStationId(removeStationIds[index]);
        stationIdStatusTable->RemoveElement(removeStationIds[index]);
    }
}

void JointSessionJob::RecordMonitoringDataWhenProcessFailure()
{
    common::g_sessionBeginMonitoringContent.m_metric128 =
        g_piaSession->GetJointSessionStateForDebug();
}

void JointSessionJob::NoticeHostChangeEventImpl(unsigned long long hostStationId)
{
}

void JointSessionJob::NoticeOwnerChangeEventImpl(unsigned int sessionId,
                                                 unsigned long long ownerStationId)
{
}

void JointSessionJob::NoticeLeaveEventImpl(unsigned int sessionId,
                                           unsigned long long stationId)
{
}

void JointSessionJob::NoticeUnregisterEventImpl(unsigned int sessionId)
{
}

}}}
