#include "nn/pia/lan/LanMatchJointSessionJob.hpp"

namespace nn { namespace pia { namespace lan {
namespace {
typedef common::Job::ExecuteResult (LanMatchJointSessionJob::*Step)();

enum LanMatchResult
{
    cResultSessionDisconnected = 0xc406,
    cResultNotSupported = 0xc0b,
    cResultInvalidArgument = 0x10c07
};

enum
{
    cSessionStatusDisconnected = 5
};

inline void SelectStep(common::StepSequenceJob::Step& slot, Step step)
{
    union { Step derived; common::Job::ExecuteResult (common::StepSequenceJob::*base)(); } value;
    value.derived = step;
    slot.m_function = value.base;
}

inline bool FinishIfSessionDisconnected(LanMatchJointSessionJob& job)
{
    if (session::g_piaSession->GetStatus() != cSessionStatusDisconnected)
        return false;
    job.ClearProcessTypeAndNoticeFailureEvent();
    if (job.m_callerContext != 0)
    {
        job.m_callerContext->SignalFailure(cResultSessionDisconnected);
        job.m_callerContext = 0;
    }
    return true;
}
}

LanMatchJointSessionJob::LanMatchJointSessionJob()
    : session::JointSessionJob(), m_stationConnectionInfo()
{
}

LanMatchJointSessionJob::~LanMatchJointSessionJob()
{
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::CallSessionEvent()
{
    NoticeStartingEventByProcessType();
    session::g_piaSession->m_postponeNotice = true;
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitNextSessionId);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitNextSessionId";
    switch (m_processType)
    {
    case session::Session::cProcessTypeJoinJointSession:
        if (m_field9c != 0)
        {
            SelectStep(m_currentStep, &LanMatchJointSessionJob::StartJoinMatchmakeSession);
            m_currentStep.m_name = "LanMatchJointSessionJob::StartJoinMatchmakeSession";
        }
        break;
    case session::Session::cProcessTypeJoinRandomJointSession:
        if (m_field9c != 0)
        {
            SelectStep(m_currentStep, &LanMatchJointSessionJob::StartRandomMatchmake);
            m_currentStep.m_name = "LanMatchJointSessionJob::StartRandomMatchmake";
        }
        break;
    case session::Session::cProcessTypeDestroyJointSession:
    case session::Session::cProcessTypeLeaveJointSession:
        if (m_field9c == 0)
        {
            SelectStep(m_currentStep, &LanMatchJointSessionJob::SendAnswerToInvitation);
            m_currentStep.m_name = "LanMatchJointSessionJob::SendAnswerToInvitation";
        }
        else
        {
            SelectStep(m_currentStep, &LanMatchJointSessionJob::SendInvitationAsCompanion);
            m_currentStep.m_name = "LanMatchJointSessionJob::SendInvitationAsCompanion";
        }
        break;
    default:
        break;
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::SendInvitationAsCompanion()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitForAnswerToInvitation);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitForAnswerToInvitation";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitForAnswerToInvitation()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartRandomMatchmake()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitRandomMatchmake);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitRandomMatchmake";
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitRandomMatchmake()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::SendNextSessionId);
    m_currentStep.m_name = "LanMatchJointSessionJob::SendNextSessionId";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartJoinMatchmakeSession()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitJoinMatchmakeSession);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitJoinMatchmakeSession";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitJoinMatchmakeSession()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::SendNextSessionId);
    m_currentStep.m_name = "LanMatchJointSessionJob::SendNextSessionId";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::SendNextSessionId()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitCompanionStationPrepared);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitCompanionStationPrepared";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitCompanionStationPrepared()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::StartLeavePreviousMesh);
    m_currentStep.m_name = "LanMatchJointSessionJob::StartLeavePreviousMesh";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::SendAnswerToInvitation()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitNextSessionId);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitNextSessionId";
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitNextSessionId()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::StartUpdateSessionKey);
    m_currentStep.m_name = "LanMatchJointSessionJob::StartUpdateSessionKey";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartUpdateSessionKey()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitUpdateSessionKey);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitUpdateSessionKey";
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitUpdateSessionKey()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::SendPreparedForMigrateSession);
    m_currentStep.m_name = "LanMatchJointSessionJob::SendPreparedForMigrateSession";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::SendPreparedForMigrateSession()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitUntilLeaderLeavesMesh);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitUntilLeaderLeavesMesh";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitUntilLeaderLeavesMesh()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::StartLeavePreviousMesh);
    m_currentStep.m_name = "LanMatchJointSessionJob::StartLeavePreviousMesh";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartLeavePreviousMesh()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitLeavePreviousMesh);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitLeavePreviousMesh";
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitLeavePreviousMesh()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::StartLeavePreviousMatchmakeSession);
    m_currentStep.m_name = "LanMatchJointSessionJob::StartLeavePreviousMatchmakeSession";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartLeavePreviousMatchmakeSession()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitLeavePreviousMatchmakeSession);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitLeavePreviousMatchmakeSession";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitLeavePreviousMatchmakeSession()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::MeshRestart);
    m_currentStep.m_name = "LanMatchJointSessionJob::MeshRestart";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::MeshRestart()
{
    if (m_field9d == 0)
    {
        SelectStep(m_currentStep, &LanMatchJointSessionJob::StartGetNextMeshHostStationConnectionInfo);
        m_currentStep.m_name = "LanMatchJointSessionJob::StartGetNextMeshHostStationConnectionInfo";
    }
    else
    {
        SelectStep(m_currentStep, &LanMatchJointSessionJob::StartCreateNextMesh);
        m_currentStep.m_name = "LanMatchJointSessionJob::StartCreateNextMesh";
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartCreateNextMesh()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitCreateNextMesh);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitCreateNextMesh";
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitCreateNextMesh()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitCompanionStation);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitCompanionStation";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartGetNextMeshHostStationConnectionInfo()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitGetNextMeshHostStationConnectionInfo);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitGetNextMeshHostStationConnectionInfo";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitGetNextMeshHostStationConnectionInfo()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::StartJoinNextMesh);
    m_currentStep.m_name = "LanMatchJointSessionJob::StartJoinNextMesh";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::StartJoinNextMesh()
{
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitJoinNextMesh);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitJoinNextMesh";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitJoinNextMesh()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitCompanionStation);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitCompanionStation";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitCompanionStation()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::WaitHostStationId);
    m_currentStep.m_name = "LanMatchJointSessionJob::WaitHostStationId";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::WaitHostStationId()
{
    if (FinishIfSessionDisconnected(*this))
        return MakeExecuteResult(cExecuteKindFinished);
    SelectStep(m_currentStep, &LanMatchJointSessionJob::ProcessComplete);
    m_currentStep.m_name = "LanMatchJointSessionJob::ProcessComplete";
    return MakeExecuteResult(cExecuteKindContinue);
}

LanMatchJointSessionJob::ExecuteResult LanMatchJointSessionJob::ProcessComplete()
{
    ClearProcessTypeAndNoticeEndingEvent();
    session::g_piaSession->m_postponeNotice = false;
    if (m_callerContext != 0)
    {
        m_callerContext->SignalSuccess(0);
        m_callerContext = 0;
    }
    return MakeExecuteResult(cExecuteKindFinished);
}

void LanMatchJointSessionJob::Trace(unsigned long long value) const
{
}

Result LanMatchJointSessionJob::StartupJoinRandomJointSessionImpl(
    const session::JoinRandomSessionSetting* setting)
{
    return cResultNotSupported;
}

Result LanMatchJointSessionJob::StartupCreateJointSessionImpl(
    const session::CreateSessionSetting* setting)
{
    return cResultNotSupported;
}

Result LanMatchJointSessionJob::StartupJoinJointSessionImpl(
    const session::JoinSessionSetting* setting)
{
    return cResultNotSupported;
}

Result LanMatchJointSessionJob::StartupLeaveJointSessionImpl()
{
    return cResultNotSupported;
}

Result LanMatchJointSessionJob::StartupDestroyJointSessionImpl()
{
    return cResultNotSupported;
}

Result LanMatchJointSessionJob::StartupPassiveProcessImpl(
    session::Session::ProcessType processType,
    session::SessionProtocol::MessageType messageType,
    const unsigned long long* stationIds,
    unsigned short stationIdNum,
    unsigned long long senderStationId)
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

    const unsigned long long localStationId = session::g_piaSession->m_localStationId;
    bool senderFound = false;
    bool localFound = false;
    if (stationIdNum != 0)
    {
        do
        {
            common::ListNode* freeNode = m_companionStationIdFreeList.PopBackNode();
            if (freeNode != 0)
            {
                StationIdNode* stationIdNode = reinterpret_cast<StationIdNode*>(
                    reinterpret_cast<char*>(freeNode) - m_stationIdNodeOffset);
                stationIdNode->m_stationId = 0;
                m_companionStationIdList.InsertBeforeNode(sentinel, stationIdNode);
                stationIdNode->m_stationId = *stationIds;
                if (*stationIds == senderStationId)
                    senderFound = true;
                if (*stationIds == localStationId)
                    localFound = true;
            }
            --stationIdNum;
            ++stationIds;
        } while (stationIdNum != 0);

        if (senderFound)
        {
            if (!localFound)
                return cResultInvalidArgument;
            m_field98 = 0;
            SelectStep(m_currentStep, &LanMatchJointSessionJob::CallSessionEvent);
            m_currentStep.m_name = "LanMatchJointSessionJob::CallSessionEvent";
            return 0;
        }
    }
    return cResultInvalidArgument;
}

Result LanMatchJointSessionJob::ReceiveLeaveInvitationImpl(session::Session::ProcessType processType,
                                                           const unsigned long long* stationIds,
                                                           unsigned short stationIdNum,
                                                           unsigned long long senderStationId)
{
    return cResultNotSupported;
}

unsigned int LanMatchJointSessionJob::GetJointSessionStateForDebug() const
{
    return 0;
}

Result LanMatchJointSessionJob::ReceiveNextSessionIdImpl(session::Session::ProcessType processType,
                                                         unsigned int sessionId,
                                                         const unsigned char* applicationData,
                                                         unsigned char applicationDataSize,
                                                         unsigned long long senderStationId)
{
    return 0;
}

Result LanMatchJointSessionJob::ReceiveLeaveMeshInvitationImpl(session::Session::ProcessType processType,
                                                               unsigned long long senderStationId)
{
    return 0;
}

Result LanMatchJointSessionJob::ReceiveCompletionResponseImpl(session::Session::ProcessType processType,
                                                              unsigned long long senderStationId)
{
    return 0;
}

void LanMatchJointSessionJob::CleanupImpl()
{
}

}}}
