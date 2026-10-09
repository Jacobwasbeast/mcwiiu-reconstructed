#include "nn/pia/inet/NexMatchMeshLayerController.hpp"

#include <coreinit/memory.h>

#include "nn/nex/NgsBridgeInterface.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/pia/NetworkFactory.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/nex/NotificationEvent.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace inet {

namespace
{

enum
{
    cNexResultNotConnected = static_cast<int>(0x800300D2u),
    cResultBufferFull = 0x4c0d,
    cResultOwnerChanged = 0x6c59,
    cNotificationTypeCategoryUnit = 1000,
    cNotificationTypeParticipation = 3,
    cNotificationTypeOwnershipChange = 4,
    cNotificationTypeGatheringUnregistered = 109,
    cNotificationTypeHostChange = 110,
    cNotificationTypeSystemPasswordChange = 120,
    cNotificationTypeSystemPasswordClear = 121,
    cNotificationTypeSwitchGathering = 122,
    cParticipationEventParticipate = 1,
    cParticipationEventDisconnect = 7,
    cParticipationEventEndParticipation = 8,
    cParticipantTypePlayer = 1,
    cDecimalDigitNum = 10,
    cParticipationTextStationIdLength = 67,
    cParticipationTextStationIdOffset = 2,
    cStationIdTextLength = 16,
    cMatchmakeStatusJointSessionProcessing = 3,
    cExpiredParticipantTimeoutMsec = 500
};

}

NexMatchMeshLayerController::NexNotificationEventHandler::NexNotificationEventHandler()
    : m_unregisterGathering(false), m_sessionParticipantInfo(), m_ignoredSessionId()
{
    AllClearSessionId();
    AllClearIgnoredSessionId();
}

NexMatchMeshLayerController::NexNotificationEventHandler::~NexNotificationEventHandler()
{
    Cleanup();
}

void NexMatchMeshLayerController::NexNotificationEventHandler::AllClearSessionId()
{
    for (unsigned int index = 0; index < cSessionIdArraySize; ++index)
        m_sessionParticipantInfo[index].Cleanup();
}

void NexMatchMeshLayerController::NexNotificationEventHandler::AllClearIgnoredSessionId()
{
    m_ignoredSessionId.m_num = 0;
    m_ignoredSessionId.m_head = 0;
}

void NexMatchMeshLayerController::NexNotificationEventHandler::Cleanup()
{
    m_unregisterGathering = false;
    AllClearSessionId();
    AllClearIgnoredSessionId();
}

bool NexMatchMeshLayerController::NexNotificationEventHandler::IsUnregisterGathering() const
{
    return m_unregisterGathering;
}

bool NexMatchMeshLayerController::NexNotificationEventHandler::IsIgnoredSessionId(unsigned int sessionId) const
{
    for (unsigned int index = 0; index < m_ignoredSessionId.m_num; ++index)
    {
        const unsigned int ignoredSessionId =
            m_ignoredSessionId.m_sessionId[(m_ignoredSessionId.m_head + index) % cIgnoredSessionIdNum];
        if (ignoredSessionId != 0 && ignoredSessionId == sessionId)
            return true;
    }
    return false;
}

void NexMatchMeshLayerController::NexNotificationEventHandler::AddIgnoredSessionId(unsigned int sessionId)
{
    if (IsIgnoredSessionId(sessionId))
        return;
    m_ignoredSessionId.m_sessionId[(m_ignoredSessionId.m_head + m_ignoredSessionId.m_num) % cIgnoredSessionIdNum] =
        sessionId;
    if (m_ignoredSessionId.m_num == cIgnoredSessionIdNum)
        m_ignoredSessionId.m_head = (m_ignoredSessionId.m_head + 1) % cIgnoredSessionIdNum;
    else
        ++m_ignoredSessionId.m_num;
}

void NexMatchMeshLayerController::NexNotificationEventHandler::RemoveIgnoredSessionId(unsigned int sessionId)
{
    if (!IsIgnoredSessionId(sessionId))
        return;
    for (unsigned int index = 0; index < m_ignoredSessionId.m_num; ++index)
    {
        unsigned int& ignoredSessionId =
            m_ignoredSessionId.m_sessionId[(m_ignoredSessionId.m_head + index) % cIgnoredSessionIdNum];
        if (ignoredSessionId == sessionId)
        {
            ignoredSessionId = 0;
            return;
        }
    }
}

Result NexMatchMeshLayerController::NexNotificationEventHandler::SaveSessionId(unsigned int sessionId)
{
    unsigned int emptyIndex = cSessionIdArraySize;
    for (unsigned int index = 0; index < cSessionIdArraySize; ++index)
    {
        if (m_sessionParticipantInfo[index].m_sessionId == sessionId)
            return transport::cResultAlreadyExists;
        if (emptyIndex == cSessionIdArraySize && m_sessionParticipantInfo[index].m_sessionId == 0)
            emptyIndex = index;
    }
    if (emptyIndex == cSessionIdArraySize)
        return cResultBufferFull;
    m_sessionParticipantInfo[emptyIndex].m_sessionId = sessionId;
    m_sessionParticipantInfo[emptyIndex].m_participantList.Clear();
    return 0;
}

bool NexMatchMeshLayerController::NexNotificationEventHandler::CheckSessionId(unsigned int sessionId) const
{
    for (unsigned int index = 0; index < cSessionIdArraySize; ++index)
    {
        if (m_sessionParticipantInfo[index].m_sessionId == sessionId)
            return true;
    }
    return false;
}

void NexMatchMeshLayerController::NexNotificationEventHandler::ClearSessionId(unsigned int sessionId)
{
    for (unsigned int index = 0; index < cSessionIdArraySize; ++index)
    {
        if (m_sessionParticipantInfo[index].m_sessionId == sessionId)
            m_sessionParticipantInfo[index].Cleanup();
    }
    AddIgnoredSessionId(sessionId);
}

unsigned int NexMatchMeshLayerController::NexNotificationEventHandler::GetSessionId(unsigned int index) const
{
    if (index >= cSessionIdArraySize)
        return 0;
    return m_sessionParticipantInfo[index].m_sessionId;
}

SessionParticipantInfo* NexMatchMeshLayerController::NexNotificationEventHandler::GetSessionParticipantInfo(
    unsigned int sessionId)
{
    for (unsigned int index = 0; index < cSessionIdArraySize; ++index)
    {
        if (m_sessionParticipantInfo[index].m_sessionId == sessionId)
            return &m_sessionParticipantInfo[index];
    }
    return 0;
}

const SessionParticipantInfo* NexMatchMeshLayerController::NexNotificationEventHandler::GetSessionParticipantInfo(
    unsigned int sessionId) const
{
    for (unsigned int index = 0; index < cSessionIdArraySize; ++index)
    {
        if (m_sessionParticipantInfo[index].m_sessionId == sessionId)
            return &m_sessionParticipantInfo[index];
    }
    return 0;
}

unsigned short NexMatchMeshLayerController::NexNotificationEventHandler::GetMatchmakeSessionTotalStations(
    unsigned int sessionId) const
{
    unsigned short stationNum = 0;
    if (session::g_piaSession != 0)
    {
        const SessionParticipantInfo* info = GetSessionParticipantInfo(sessionId);
        if (info != 0)
        {
            const common::ListNode* end = info->m_participantList.Sentinel();
            for (const common::ListNode* node = end->next; node != end; node = node->next)
            {
                const SessionParticipantInfo::Participant* participant =
                    SessionParticipantInfo::ParticipantList::ValueOf(node);
                if (participant->m_principalId != 0 && participant->m_principalId == participant->m_stationId &&
                    participant->m_participationCount > 0)
                    ++stationNum;
            }
        }
    }
    return stationNum;
}

unsigned short NexMatchMeshLayerController::NexNotificationEventHandler::GetMatchmakeSessionTotalParticipants(
    unsigned int sessionId) const
{
    unsigned short participantNum = 0;
    if (session::g_piaSession != 0)
    {
        const SessionParticipantInfo* info = GetSessionParticipantInfo(sessionId);
        if (info != 0)
        {
            const common::ListNode* end = info->m_participantList.Sentinel();
            for (const common::ListNode* node = end->next; node != end; node = node->next)
            {
                const SessionParticipantInfo::Participant* participant =
                    SessionParticipantInfo::ParticipantList::ValueOf(node);
                if (participant->m_principalId != 0 && participant->m_participationCount > 0 &&
                    participant->m_participantType == cParticipantTypePlayer)
                    ++participantNum;
            }
        }
    }
    return participantNum;
}

bool NexMatchMeshLayerController::NexNotificationEventHandler::IsParticipated(unsigned int sessionId,
                                                                              unsigned long long principalId) const
{
    bool participated = false;
    const SessionParticipantInfo* info = GetSessionParticipantInfo(sessionId);
    if (info != 0)
    {
        const common::ListNode* end = info->m_participantList.Sentinel();
        for (const common::ListNode* node = end->next; node != end; node = node->next)
        {
            const SessionParticipantInfo::Participant* participant =
                SessionParticipantInfo::ParticipantList::ValueOf(node);
            if (participant->m_principalId == principalId && participant->m_participationCount > 0)
            {
                participated = true;
                break;
            }
        }
    }
    return participated;
}

void NexMatchMeshLayerController::NexNotificationEventHandler::AddParticipantPrincipalId(
    unsigned int sessionId, unsigned long long principalId, unsigned long long stationId,
    unsigned char participantType)
{
    SessionParticipantInfo* info = GetSessionParticipantInfo(sessionId);
    if (info == 0)
    {
        if (SaveSessionId(sessionId) != 0)
            return;
        info = GetSessionParticipantInfo(sessionId);
        if (info == 0)
            return;
    }
    SessionParticipantInfo::ParticipantList& list = info->m_participantList;
    common::ListNode* end = list.Sentinel();
    common::ListNode* node = end->next;
    common::ListNode* next = node->next;
    while (node != end)
    {
        SessionParticipantInfo::ParticipantList::Node* participantNode =
            static_cast<SessionParticipantInfo::ParticipantList::Node*>(node);
        if (static_cast<long long>(common::g_piaJobScheduler->m_dispatchTime.m_tick -
                                   participantNode->value.m_updateTime.m_tick) >
                common::TimeSpan::GetTicksPerMSec().m_tick * cExpiredParticipantTimeoutMsec &&
            participantNode->value.m_participationCount < 0 &&
            (participantNode->value.m_participationEvent == cParticipationEventDisconnect ||
             participantNode->value.m_participationEvent == cParticipationEventEndParticipation))
            list.Free(participantNode);
        node = next;
        next = node->next;
    }
    for (node = end->next; node != end; node = node->next)
    {
        SessionParticipantInfo::Participant* participant = SessionParticipantInfo::ParticipantList::ValueOf(node);
        if (participant->m_principalId == principalId)
        {
            participant->m_stationId = stationId;
            participant->m_participantType = participantType;
            if (participant->m_participationEvent == cParticipationEventDisconnect)
            {
                participant->m_participationCount = 1;
                participant->m_participationEvent = cParticipationEventParticipate;
                participant->m_updateTime = common::g_piaJobScheduler->m_dispatchTime;
                return;
            }
            participant->m_participationEvent = cParticipationEventParticipate;
            ++participant->m_participationCount;
            participant->m_updateTime = common::g_piaJobScheduler->m_dispatchTime;
            if (participant->m_participationCount == 0)
                list.Free(static_cast<SessionParticipantInfo::ParticipantList::Node*>(node));
            return;
        }
    }
    SessionParticipantInfo::Participant* participant = list.Alloc();
    if (participant != 0)
    {
        participant->m_principalId = principalId;
        participant->m_stationId = stationId;
        participant->m_participationEvent = cParticipationEventParticipate;
        participant->m_participantType = participantType;
        participant->m_participationCount = 1;
        participant->m_leaving = false;
        participant->m_updateTime = common::g_piaJobScheduler->m_dispatchTime;
    }
}

void NexMatchMeshLayerController::NexNotificationEventHandler::RemoveParticipantPrincipalId(
    unsigned int sessionId, unsigned long long principalId, short participationEvent)
{
    SessionParticipantInfo* info = GetSessionParticipantInfo(sessionId);
    if (info == 0)
    {
        if (IsIgnoredSessionId(sessionId))
            return;
        if (SaveSessionId(sessionId) != 0)
            return;
        info = GetSessionParticipantInfo(sessionId);
        if (info == 0)
            return;
    }
    SessionParticipantInfo::ParticipantList& list = info->m_participantList;
    common::ListNode* end = list.Sentinel();
    common::ListNode* node = end->next;
    common::ListNode* next = node->next;
    while (node != end)
    {
        SessionParticipantInfo::ParticipantList::Node* participantNode =
            static_cast<SessionParticipantInfo::ParticipantList::Node*>(node);
        if (static_cast<long long>(common::g_piaJobScheduler->m_dispatchTime.m_tick -
                                   participantNode->value.m_updateTime.m_tick) >
                common::TimeSpan::GetTicksPerMSec().m_tick * cExpiredParticipantTimeoutMsec &&
            participantNode->value.m_participationCount < 0 &&
            (participantNode->value.m_participationEvent == cParticipationEventDisconnect ||
             participantNode->value.m_participationEvent == cParticipationEventEndParticipation))
            list.Free(participantNode);
        node = next;
        next = node->next;
    }
    for (node = end->next; node != end; node = node->next)
    {
        SessionParticipantInfo::Participant* participant = SessionParticipantInfo::ParticipantList::ValueOf(node);
        if (participant->m_principalId == principalId)
        {
            if (participationEvent == cParticipationEventEndParticipation &&
                participant->m_participationEvent == cParticipationEventDisconnect)
            {
                participant->m_participationEvent = participationEvent;
                participant->m_updateTime = common::g_piaJobScheduler->m_dispatchTime;
                return;
            }
            participant->m_participationEvent = participationEvent;
            --participant->m_participationCount;
            participant->m_updateTime = common::g_piaJobScheduler->m_dispatchTime;
            if (participant->m_participationCount == 0)
                list.Free(static_cast<SessionParticipantInfo::ParticipantList::Node*>(node));
            return;
        }
    }
    SessionParticipantInfo::Participant* participant = list.Alloc();
    if (participant != 0)
    {
        participant->m_principalId = principalId;
        participant->m_stationId = 0;
        participant->m_participationEvent = participationEvent;
        participant->m_participationCount = -1;
        participant->m_leaving = false;
        participant->m_updateTime = common::g_piaJobScheduler->m_dispatchTime;
    }
}

void NexMatchMeshLayerController::NexNotificationEventHandler::ProcessNotificationEvent(
    const nex::NotificationEvent& event)
{
    const unsigned int eventCategory = event.GetType() / cNotificationTypeCategoryUnit;
    if (eventCategory != cNotificationTypeParticipation && eventCategory != cNotificationTypeOwnershipChange &&
        eventCategory != cNotificationTypeGatheringUnregistered && eventCategory != cNotificationTypeHostChange &&
        eventCategory != cNotificationTypeSystemPasswordChange &&
        eventCategory != cNotificationTypeSystemPasswordClear && eventCategory != cNotificationTypeSwitchGathering)
        return;

    session::Session* piaSession = session::g_piaSession;
    const unsigned short eventType = static_cast<unsigned short>(eventCategory);
    const unsigned int eventSubType = event.GetType() % cNotificationTypeCategoryUnit;
    const unsigned int sessionId = event.GetParam1();
    const unsigned long long principalId = event.GetParam2();
    NexMatchmakeSession* currentSession =
        static_cast<NexMatchmakeSession*>(piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]);
    NexMatchmakeSession* bufferSession =
        static_cast<NexMatchmakeSession*>(piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex == 0]);
    const unsigned long long localPrincipalId = NexFacade::GetInstance()->GetLocalPrincipalId();

    if (!IsIgnoredSessionId(sessionId))
    {
        SaveSessionId(sessionId);
    }
    else
    {
        if (principalId != localPrincipalId)
            return;
        if (eventType == cNotificationTypeParticipation)
        {
            if (eventSubType != cParticipationEventParticipate)
                return;
        }
        else if (eventType != cNotificationTypeSwitchGathering)
        {
            return;
        }
        RemoveIgnoredSessionId(sessionId);
    }

    switch (eventType)
    {
    case cNotificationTypeParticipation:
        if (eventSubType == cParticipationEventParticipate)
        {
            unsigned char participantType = cParticipantTypePlayer;
            const char* text = reinterpret_cast<const char*>(static_cast<const wchar_t*>(event.m_StringParam));
            unsigned int length = event.m_StringParam.GetLength();
            if (length != 0)
            {
                const unsigned char type = static_cast<unsigned char>(text[0] - '0');
                if (static_cast<unsigned int>(text[0] - '0') < cDecimalDigitNum && type >= cParticipantTypePlayer &&
                    type <= cParticipantTypePlayer)
                    participantType = type;
                if (length >= cParticipationTextStationIdLength)
                {
                    unsigned long long stationId;
                    length -= cParticipationTextStationIdOffset;
                    text += cParticipationTextStationIdOffset;
                    if (length >= cStationIdTextLength)
                    {
                        char stationIdText[cStationIdTextLength + 1];
                        OSBlockMove(stationIdText, text, cStationIdTextLength, FALSE);
                        stationIdText[cStationIdTextLength] = 0;
                        pead::StringUtil::tryParseU64(&stationId, pead::SafeStringBase<char>(stationIdText),
                                                      pead::StringUtil::cHexadecimal);
                    }
                    if (principalId != stationId)
                        participantType = cParticipantTypePlayer;
                    AddParticipantPrincipalId(sessionId, principalId, stationId, participantType);
                    return;
                }
            }
            AddParticipantPrincipalId(sessionId, principalId, principalId, participantType);
            return;
        }
        RemoveParticipantPrincipalId(sessionId, principalId, static_cast<short>(eventSubType));
        piaSession->NotifyLeave(sessionId, principalId);
        if (principalId == localPrincipalId)
        {
            ClearSessionId(sessionId);
            AddIgnoredSessionId(sessionId);
        }
        break;

    case cNotificationTypeOwnershipChange:
        if (piaSession->m_sessionId[piaSession->m_currentSessionIndex] == sessionId)
        {
            currentSession->SetMatchmakeOwner(principalId == localPrincipalId);
            currentSession->SetMatchmakeOwnerUniqueId(principalId);
        }
        else if (piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0] == sessionId)
        {
            if (bufferSession != 0)
            {
                bufferSession->SetMatchmakeOwner(principalId == localPrincipalId);
                bufferSession->SetMatchmakeOwnerUniqueId(principalId);
            }
        }
        else if (currentSession->IsProcessingJoinMatchmakeSession())
        {
            currentSession->SetTemporaryOwnerChangeEvent(sessionId, principalId);
        }
        else if (bufferSession->IsProcessingJoinMatchmakeSession())
        {
            bufferSession->SetTemporaryOwnerChangeEvent(sessionId, principalId);
        }
        piaSession->NotifyOwnershipChange(sessionId, principalId);
        break;

    case cNotificationTypeGatheringUnregistered:
        if (piaSession->m_sessionId[piaSession->m_currentSessionIndex] == sessionId)
            m_unregisterGathering = true;
        if (piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0] == sessionId && bufferSession != 0)
            m_unregisterGathering = true;
        ClearSessionId(sessionId);
        AddIgnoredSessionId(sessionId);
        piaSession->NotifyUnregister(sessionId);
        break;

    case cNotificationTypeSystemPasswordChange:
        if (sessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex] ||
            sessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0])
        {
            common::String password(static_cast<const wchar_t*>(event.m_StringParam));
            piaSession->SetSessionSystemPassword(password);
            session::Session::NoticeEventCallback(session::Session::cEventTypeSessionSystemPasswordSet,
                                                  cStationIndexInvalid);
        }
        break;

    case cNotificationTypeSystemPasswordClear:
        if (sessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex] ||
            sessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0])
        {
            piaSession->ResetSessionSystemPassword();
            session::Session::NoticeEventCallback(session::Session::cEventTypeSessionSystemPasswordClear,
                                                  cStationIndexInvalid);
        }
        break;

    case cNotificationTypeSwitchGathering:
        if (principalId == localPrincipalId)
            SaveSessionId(sessionId);
        if (principalId == localPrincipalId &&
            piaSession->m_sessionId[piaSession->m_currentSessionIndex] != sessionId && bufferSession != 0 &&
            piaSession->m_matchmakeStatus == cMatchmakeStatusJointSessionProcessing)
        {
            const unsigned int bufferSessionId = piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0];
            if (bufferSessionId == 0 || bufferSessionId == sessionId)
            {
                session::g_piaSession->m_meshLayerController->SetBufferSessionId(sessionId);
            }
            else if (piaSession->m_meshLayerController->GetJointSessionJobPtr() != 0 &&
                     piaSession->m_meshLayerController->GetJointSessionJobPtr()->IsRunning())
            {
                session::JointSessionJob* jointSessionJob =
                    static_cast<session::JointSessionJob*>(piaSession->m_meshLayerController->GetJointSessionJobPtr());
                jointSessionJob->m_failurePending = true;
                jointSessionJob->m_pendingFailureResult = cResultOwnerChanged;
            }
        }
        break;

    default:
        break;
    }
}

NexMatchMeshLayerController::NexMatchMeshLayerController()
    : m_nexFacadeNgsInterface(0)
{
    void* storage = pead::allocateFromSeadHeap(sizeof(NexNotificationEventHandler), common::HeapManager::GetHeap());
    m_notificationEventHandler = storage != 0 ? ::new (storage) NexNotificationEventHandler() : 0;
}

NexMatchMeshLayerController::~NexMatchMeshLayerController()
{
    if (m_notificationEventHandler != 0)
    {
        transport::DestroyObject(m_notificationEventHandler);
        m_notificationEventHandler = 0;
    }
}

Result NexMatchMeshLayerController::Startup(const session::Session::StartupSetting& setting)
{
    return MeshLayerController::Startup(setting);
}

void NexMatchMeshLayerController::MeshCleanup()
{
    MeshLayerController::MeshCleanup();
    NexFacade::GetInstance()->StopNatSession();
    NexFacade::GetInstance()->Cleanup();
}

Result NexMatchMeshLayerController::MeshStartup(session::IMatchmakeSession* matchmakeSession, bool skipBandwidthCheck)
{
    return MeshLayerController::MeshStartup(matchmakeSession, skipBandwidthCheck);
}

Result NexMatchMeshLayerController::StartNatSession()
{
    Result result = NexFacade::GetInstance()->Startup();
    if (result == 0)
    {
        result = NexFacade::GetInstance()->StartNatSessionAsync();
        if (result == 0)
            return result;
        NexFacade::GetInstance()->StopNatSession();
    }
    NexFacade::GetInstance()->Cleanup();
    return result;
}

common::Job::ExecuteKind NexMatchMeshLayerController::WaitStartNatSession(Result* result)
{
    if (NexFacade::GetInstance()->IsStartNatSessionCompleted())
    {
        const Result startResult = NexFacade::GetInstance()->GetStartNatSessionResult();
        if (startResult == 0)
        {
            session::Session* piaSession = session::g_piaSession;
            NexFacade::GetInstance()->UpdateGatheringID(piaSession->m_sessionId[piaSession->m_currentSessionIndex]);
            *result = startResult;
            return common::Job::cExecuteKindContinue;
        }
        *result = startResult;
        NexFacade::GetInstance()->StopNatSession();
        NexFacade::GetInstance()->Cleanup();
        return common::Job::cExecuteKindFinished;
    }
    return common::Job::cExecuteKindNextStep;
}

Result NexMatchMeshLayerController::CancelStartNatSession()
{
    return NexFacade::GetInstance()->CancelStartNatSessionAsync();
}

void NexMatchMeshLayerController::SetLeaveFlag()
{
    NexFacade::GetInstance()->SetIgnoreNatProbeRequest(true);
}

bool NexMatchMeshLayerController::IsAllStationJoined(unsigned int sessionId)
{
    const SessionParticipantInfo* info = GetSessionParticipantInfo(sessionId);
    transport::StationIdTable* stationIdTable = transport::g_piaTransport->m_stationIdTable;
    const common::ListNode* end = info->m_participantList.Sentinel();
    for (const common::ListNode* node = end->next; node != end; node = node->next)
    {
        const SessionParticipantInfo::Participant* participant =
            SessionParticipantInfo::ParticipantList::ValueOf(node);
        if (participant->m_principalId == participant->m_stationId &&
            stationIdTable->FindByUniqueIdCore(participant->m_principalId) == 0)
            return false;
    }
    return true;
}

void NexMatchMeshLayerController::Cleanup()
{
    NexMatchMeshLayerController::MeshCleanup();
    m_notificationEventHandler->Cleanup();
    MeshLayerController::Cleanup();
    static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex])->Finalize();
    NexMatchmakeSession* otherMatchmakeSession = static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex == 0]);
    if (otherMatchmakeSession != 0)
        otherMatchmakeSession->Finalize();
    if (m_nexFacadeNgsInterface != 0)
    {
        m_nexFacadeNgsInterface->UnregisterNotificationEventHandler(m_notificationEventHandler);
        m_nexFacadeNgsInterface = 0;
    }
    ResetPlayerInfo();
    m_ngsInterface = 0;
    m_fielde0 = 0;
}

Result NexMatchMeshLayerController::CheckConnectedStatusCore(nex::NgsBridgeInterface* ngsInterface)
{
    if (ngsInterface == 0)
        return transport::cResultInvalidArgument;
    if (!ngsInterface->IsConnected() && ngsInterface->IsConnected() == nex::qResult(cNexResultNotConnected))
        return session::Session::cResultStatusDisconnectedNetwork;
    return 0;
}

Result NexMatchMeshLayerController::CheckConnectedStatus()
{
    if (m_ngsInterface != 0)
    {
        const Result result = CheckConnectedStatusCore(m_ngsInterface);
        if (result != 0)
            return result;
    }
    if (m_notificationEventHandler->IsUnregisterGathering())
        return session::Session::cResultStatusDisconnectedSession;
    return 0;
}

bool NexMatchMeshLayerController::CheckSessionId(unsigned int sessionId)
{
    if (m_notificationEventHandler == 0)
        return false;
    return m_notificationEventHandler->CheckSessionId(sessionId);
}

void NexMatchMeshLayerController::ClearSessionId(unsigned int sessionId)
{
    if (m_notificationEventHandler != 0)
        m_notificationEventHandler->ClearSessionId(sessionId);
}

unsigned int NexMatchMeshLayerController::GetSessionId(unsigned int index) const
{
    if (m_notificationEventHandler == 0)
        return 0;
    return m_notificationEventHandler->GetSessionId(index);
}

unsigned int NexMatchMeshLayerController::GetSessionIdArraySize() const
{
    if (m_notificationEventHandler == 0)
        return 0;
    return cSessionIdArraySize;
}

const SessionParticipantInfo* NexMatchMeshLayerController::GetSessionParticipantInfo(unsigned int sessionId) const
{
    if (m_notificationEventHandler == 0)
        return 0;
    return m_notificationEventHandler->GetSessionParticipantInfo(sessionId);
}

unsigned int NexMatchMeshLayerController::GetMatchmakeSessionTotalStations() const
{
    session::Session* piaSession = session::g_piaSession;
    if (m_notificationEventHandler == 0 || piaSession == 0)
        return 0;
    if (piaSession->GetJointSessionId() != 0)
        return m_notificationEventHandler->GetMatchmakeSessionTotalStations(piaSession->GetJointSessionId());
    return m_notificationEventHandler->GetMatchmakeSessionTotalStations(
        piaSession->m_sessionId[piaSession->m_currentSessionIndex]);
}

unsigned int NexMatchMeshLayerController::GetMatchmakeSessionTotalParticipants() const
{
    session::Session* piaSession = session::g_piaSession;
    if (m_notificationEventHandler == 0 || piaSession == 0)
        return 0;
    if (piaSession->GetJointSessionId() != 0)
        return m_notificationEventHandler->GetMatchmakeSessionTotalParticipants(piaSession->GetJointSessionId());
    return m_notificationEventHandler->GetMatchmakeSessionTotalParticipants(
        piaSession->m_sessionId[piaSession->m_currentSessionIndex]);
}

bool NexMatchMeshLayerController::IsParticipated(unsigned int sessionId, unsigned long long stationId) const
{
    if (m_notificationEventHandler == 0)
        return false;
    return m_notificationEventHandler->IsParticipated(sessionId, stationId);
}

unsigned int NexMatchMeshLayerController::GetMatchmakeSessionTotalStations(unsigned int sessionId) const
{
    if (m_notificationEventHandler == 0 || session::g_piaSession == 0)
        return 0;
    return m_notificationEventHandler->GetMatchmakeSessionTotalStations(sessionId);
}

void NexMatchMeshLayerController::CleanupEventHandler()
{
    if (m_notificationEventHandler != 0)
        m_notificationEventHandler->Cleanup();
}

bool NexMatchMeshLayerController::IsJoinedUnknownSession(unsigned int sessionId, unsigned int bufferSessionId)
{
    for (unsigned int index = 0; index < GetSessionIdArraySize(); ++index)
    {
        const unsigned int joinedSessionId = GetSessionId(index);
        if (joinedSessionId != 0 && joinedSessionId != sessionId && joinedSessionId != bufferSessionId)
            return true;
    }
    return false;
}

nex::NgsBridgeInterface* NexMatchMeshLayerController::GetNgsInterface(unsigned char index)
{
    if (index != 0)
        return 0;
    return m_ngsInterface;
}

unsigned char NexMatchMeshLayerController::GetNgsInterfaceIndexBoundNexFacade() const
{
    return m_ngsInterfaceIndexBoundNexFacade;
}

void NexMatchMeshLayerController::Trace(unsigned long long) const
{
}

}}}
