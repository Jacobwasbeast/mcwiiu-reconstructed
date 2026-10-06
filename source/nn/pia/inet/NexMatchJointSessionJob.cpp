#include "nn/pia/inet/NexMatchJointSessionJob.hpp"

#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn { namespace pia { namespace inet {

namespace
{
typedef common::Job::ExecuteResult ExecuteResult;
typedef ExecuteResult (NexMatchJointSessionJob::*Step)();

enum
{
    cSessionStatusDisconnectedSession = 4,
    cSessionStatusDisconnectedNetwork = 5,
    cSessionStatusUnknown6 = 6,
    cSessionStatusUnknown7 = 7,
    cSessionStatusUnknown8 = 8
};

enum
{
    cResultSessionDisconnected = 0xc406,
    cResultSessionStatus6 = 0xa471,
    cResultSessionStatus7 = 0xc420,
    cResultSessionStatus8 = 0xc421,
    cResultOwnerChanged = 0x6c59,
    cResultUpdateHostFailed = 0xac5a,
    cResultRetryJoinMesh = 0x6c4b
};

enum
{
    cFailurePointSessionStatus = 3,
    cFailurePointConnectionError = 4,
    cFailurePointDisconnectDetected = 6,
    cFailurePointNoOwnerEntry = 0x15,
    cFailurePointUpdateHostFailed = 0x17,
    cFailurePointSessionIdMismatch = 0x18,
    cFailurePointMeshStartupFailed = 0x1a,
    cFailurePointNoPrincipalId = 0x1d,
    cFailurePointCompanionTimeout = 0x23,
    cFailurePointSameSessionId = 0x24
};

enum
{
    cResultInvalidState = 0x10c08,
    cResultJoinMesh6C05 = 0x6c05,
    cResultJoinMesh6439 = 0x6439,
    cResultJoinMesh643F = 0x643f,
    cResultJoinMesh6442 = 0x6442,
    cResultJoinMesh6443 = 0x6443,
    cResultJoinMesh6444 = 0x6444,
    cResultJoinMesh6445 = 0x6445,
    cResultJoinMesh644A = 0x644a,
    cResultJoinMesh6467 = 0x6467,
    cResultJoinMesh6468 = 0x6468,
    cResultJoinMesh6469 = 0x6469,
    cResultJoinMesh646A = 0x646a,
    cResultJoinMesh6838 = 0x6838,
    cResultJoinMesh6C3D = 0x6c3d,
    cResultJoinMesh6846 = 0x6846,
    cResultJoinMesh6847 = 0x6847,
    cResultJoinMesh6848 = 0x6848,
    cResultJoinMesh6849 = 0x6849,
    cResultJoinMesh10C3E = 0x10c3e,
    cResultJoinMesh10C25 = 0x10c25
};

enum
{
    cFailurePointJoinMesh6C05 = 0x27,
    cFailurePointInvalidState = 0x28,
    cFailurePointJoinMesh6439 = 0x29,
    cFailurePointJoinMesh643F = 0x2a,
    cFailurePointJoinMesh6442 = 0x2b,
    cFailurePointJoinMesh6445 = 0x2c,
    cFailurePointJoinMesh6443 = 0x2d,
    cFailurePointJoinMesh6444 = 0x2e,
    cFailurePointJoinMesh644A = 0x2f,
    cFailurePointJoinMesh6838 = 0x30,
    cFailurePointJoinMesh6C3D = 0x31,
    cFailurePointJoinMesh6846 = 0x32,
    cFailurePointJoinMesh6847 = 0x33,
    cFailurePointJoinMesh6848 = 0x34,
    cFailurePointJoinMesh6849 = 0x35,
    cFailurePointJoinMesh10C3E = 0x36,
    cFailurePointJoinMesh10C25 = 0x37,
    cFailurePointJoinMeshOther = 0x1c
};

enum
{
    cJointSessionStateGetNextMeshHostStationConnectionInfo = 0xf,
    cJointSessionStateJoinNextMesh = 0x10,
    cJointSessionStateWaitCompanionStation = 0x11,
    cJointSessionStateSendCompletion = 0x12,
    cJointSessionStateCancelled = 0x16
};

enum
{
    cHostStationUpdateIntervalMsec = 10000,
    cRetryJoinIntervalMsec = 3000,
    cRetryJoinIntervalHostMsec = 12000,
    cCompanionTimeoutMsec = 40000,
    cCompletionTimeoutMsec = 10000,
    cJoinMeshRetryMax = 2,
    cConnectionInfoRetryMax = 2,
    cField60WaitUntilCompanionCompletion = 0x13
};

const unsigned long long cTraceJointSessionLevel = 0x0000000100000000ULL;

inline void setStep(NexMatchJointSessionJob& job, Step function, const char* name)
{
    union
    {
        Step derived;
        ExecuteResult (common::StepSequenceJob::*base)();
    } step;
    step.derived = function;
    job.m_currentStep.m_function = step.base;
    job.m_currentStep.m_name = name;
}

inline unsigned int bufferSessionIndex()
{
    return session::g_piaSession->m_currentSessionIndex == 0 ? 1 : 0;
}

inline void unregisterBufferSearchCriteria()
{
    static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[bufferSessionIndex()])->UnregisterSearchCriteria();
}

inline bool isElapsed(const common::Time& start, int msec)
{
    return static_cast<long long>(
               common::g_piaJobScheduler->m_dispatchTime.m_tick - start.m_tick) >=
           common::TimeSpan::GetTicksPerMSec().m_tick * msec;
}

inline void setDeadline(common::Time& deadline, int msec)
{
    deadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick +
                      common::TimeSpan::GetTicksPerMSec().m_tick * msec;
}

inline common::Time deadlineAfter(int msec)
{
    common::Time deadline;
    deadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick +
                      common::TimeSpan::GetTicksPerMSec().m_tick * msec;
    return deadline;
}

inline void countRetry(unsigned char& counter)
{
    if (counter == 0xff)
        counter = 1;
    else
        ++counter;
}
}

NexMatchJointSessionJob::NexMatchJointSessionJob()
    : session::JointSessionJob(), m_stationConnectionInfo(), m_failureResult(0),
      m_jointSessionState(0), m_field1ee(0), m_field270(0)
{
}

NexMatchJointSessionJob::~NexMatchJointSessionJob()
{
}

void NexMatchJointSessionJob::Trace(unsigned long long p0) const
{
}

unsigned int NexMatchJointSessionJob::GetJointSessionStateForDebug() const
{
    return m_jointSessionState;
}

Result NexMatchJointSessionJob::StartupJoinRandomJointSessionImpl(const session::JoinRandomSessionSetting* setting)
{
    return 0;
}

Result NexMatchJointSessionJob::StartupCreateJointSessionImpl(const session::CreateSessionSetting* setting)
{
    return 0;
}

Result NexMatchJointSessionJob::StartupJoinJointSessionImpl(const session::JoinSessionSetting* setting)
{
    return 0;
}

Result NexMatchJointSessionJob::StartupLeaveJointSessionImpl()
{
    return 0;
}

Result NexMatchJointSessionJob::StartupDestroyJointSessionImpl()
{
    return 0;
}

Result NexMatchJointSessionJob::StartupPassiveProcessImpl(session::Session::ProcessType processType, session::SessionProtocol::MessageType messageType, const unsigned long long* stationIds, unsigned short stationIdNum, unsigned long long senderStationId)
{
    return 0;
}

Result NexMatchJointSessionJob::ReceiveNextSessionIdImpl(session::Session::ProcessType processType, unsigned int sessionId, const unsigned char* applicationData, unsigned char applicationDataSize, unsigned long long senderStationId)
{
    return 0;
}

Result NexMatchJointSessionJob::ReceiveLeaveInvitationImpl(session::Session::ProcessType processType, const unsigned long long* stationIds, unsigned short stationIdNum, unsigned long long senderStationId)
{
    return 0;
}

Result NexMatchJointSessionJob::ReceiveLeaveMeshInvitationImpl(session::Session::ProcessType processType, unsigned long long senderStationId)
{
    return 0;
}

Result NexMatchJointSessionJob::ReceiveCompletionResponseImpl(session::Session::ProcessType processType, unsigned long long senderStationId)
{
    return 0;
}

void NexMatchJointSessionJob::NoticeHostChangeEventImpl(unsigned long long hostStationId)
{
}

void NexMatchJointSessionJob::NoticeOwnerChangeEventImpl(unsigned int sessionId, unsigned long long ownerStationId)
{
}

void NexMatchJointSessionJob::NoticeLeaveEventImpl(unsigned int sessionId, unsigned long long stationId)
{
}

void NexMatchJointSessionJob::NoticeUnregisterEventImpl(unsigned int sessionId)
{
}

void NexMatchJointSessionJob::CleanupImpl()
{
}

void NexMatchJointSessionJob::CheckJoinedOnlyCompanion()
{
}

void NexMatchJointSessionJob::CleanupMonitoringData()
{
}

void NexMatchJointSessionJob::PrepareBecomingToLeader()
{
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::CallDestroySessionEvent()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::CallSessionEvent()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::CloseJointSessionParticipation()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::CloseMatchmakeSession()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::GetMatchmakeSessionOwners()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::GetNextMatchmakeSessionInfo()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::MeshRestart()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::ProcessComplete()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::ProcessFailure()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::ResendDestroyInvitation()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::ResendInvitationAsCompanion()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::ResendNextSessionId()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendAnswerToDestroyInvitation()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendAnswerToInvitation()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendCompletionForMigrateSession()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendCompletionInvitation()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendDestroyInvitation()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendInvitationAsCompanion()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendLeaveMeshCompanion()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendNextSessionId()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::SendPreparedForMigrateSession()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartCreateMatchmakeSession()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartCreateNextMesh()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartGetNextMeshHostStationConnectionInfo()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartJoinMatchmakeSession()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartJoinNextMesh()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartLeaveBufferMatchmakeSession()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartLeaveCurrentMatchmakeSession()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartLeaveMesh()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartLeavePreviousMatchmakeSession()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartLeavePreviousMesh()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::StartRandomMatchmake()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::UpdateJointSessionOpenStatus()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitCloseJointSessionParticipation()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitCloseMatchmakeSession()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitCompanionStation()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitCompanionStationPrepared()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitCompletionInvitation()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitCreateMatchmakeSession()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitCreateNextMesh()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitForAnswerToCompletionInvitation()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitForAnswerToDestroyInvitation()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitForAnswerToInvitation()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitForInvitationAsCompanion()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitGetMatchmakeSessionOwners()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitGetNextMatchmakeSessionInfo()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitGetNextMeshHostStationConnectionInfo()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitHostMigrated()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitHostStationId()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitJoinMatchmakeSession()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitLeaveBufferMatchmakeSession()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitLeaveCurrentMatchmakeSession()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitLeaveMesh()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitLeavePreviousMatchmakeSession()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitLeavePreviousMesh()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitMeshRestart()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitNextSessionId()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitNotification()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitRandomMatchmake()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitStartRetryJoinMesh()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitUntilCompanionCompletion()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitUntilCompanionLeavesMesh()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitUntilLeaderLeavesMesh()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}

NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitUpdateJointSessionOpenStatus()
{
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}


NexMatchJointSessionJob::ExecuteResult NexMatchJointSessionJob::WaitJoinNextMesh()
{
    switch (session::g_piaSession->GetStatus())
    {
    case cSessionStatusDisconnectedNetwork:
        m_sessionDisconnectDetected = true;
        m_fieldc8 = cFailurePointSessionStatus;
        m_failureResult = cResultSessionDisconnected;
        RecordMonitoringDataWhenProcessFailure();
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        setStep(*this, &NexMatchJointSessionJob::ProcessFailure,
                "NexMatchJointSessionJob::ProcessFailure");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    case cSessionStatusUnknown6:
        m_sessionDisconnectDetected = true;
        m_fieldc8 = cFailurePointSessionStatus;
        m_failureResult = cResultSessionStatus6;
        RecordMonitoringDataWhenProcessFailure();
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        setStep(*this, &NexMatchJointSessionJob::ProcessFailure,
                "NexMatchJointSessionJob::ProcessFailure");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    case cSessionStatusUnknown7:
        m_sessionDisconnectDetected = true;
        m_fieldc8 = cFailurePointSessionStatus;
        m_failureResult = cResultSessionStatus7;
        RecordMonitoringDataWhenProcessFailure();
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        setStep(*this, &NexMatchJointSessionJob::ProcessFailure,
                "NexMatchJointSessionJob::ProcessFailure");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    case cSessionStatusUnknown8:
        m_sessionDisconnectDetected = true;
        m_fieldc8 = cFailurePointSessionStatus;
        m_failureResult = cResultSessionStatus8;
        RecordMonitoringDataWhenProcessFailure();
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        setStep(*this, &NexMatchJointSessionJob::ProcessFailure,
                "NexMatchJointSessionJob::ProcessFailure");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
    default:
        break;
    }

    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess ||
            m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        m_jointSessionState = cJointSessionStateCancelled;
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindFinished);
    }

    session::Session* session = session::g_piaSession;
    if (!session::g_piaMesh->IsJoinMeshCompleted())
    {
        if (m_fielda2 != 0)
        {
            session::g_piaMesh->CancelJoinMeshAsync();
        }
        else if (m_reserveda8[0] != 0)
        {
            session::g_piaMesh->CancelJoinMeshAsync();
        }
        else if (m_reserveda8[1] != 0)
        {
            session::g_piaMesh->CancelJoinMeshAsync();
        }

        if ((IsMigrateSessionToJointSession() && m_field1d8 != 0) ||
            (IsMigrateJointSessionToSession() && m_field1c8 != 0))
        {
            session::g_piaMesh->CancelJoinMeshAsync();
            if (m_field1e8 != 2)
                m_field1e8 = 0;
        }
    }

    if (!session::g_piaMesh->IsJoinMeshCompleted())
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);

    Result joinResult = session::g_piaMesh->GetJoinMeshResult();
    if (joinResult != 0)
    {
        if (joinResult == cResultJoinMesh643F || joinResult == cResultJoinMesh6445 ||
            joinResult == cResultJoinMesh6444 || joinResult == cResultJoinMesh6443 ||
            joinResult == cResultJoinMesh6439 || joinResult == cResultJoinMesh644A ||
            (joinResult == cResultJoinMesh6C05 && m_fielda2 == 0))
        {
            countRetry(common::g_sessionBeginMonitoringContent.m_joinRetryCount);
            ++m_field1e8;
            if (m_field1e8 < cJoinMeshRetryMax)
            {
                session::g_piaSession->m_fieldc0 = 1;
                session::g_piaSession->m_disconnectStatus = 0;
                session::g_piaMesh->Cleanup();
                if (IsMigrateSessionToJointSession() && m_field1d8 != 0 &&
                    session->m_matchmakeSession[session->m_currentSessionIndex]->IsMatchmakeOwner())
                {
                    countRetry(common::g_sessionBeginMonitoringContent.m_ownerChangeRetryCount);
                    setStep(*this, &NexMatchJointSessionJob::MeshRestart,
                            "NexMatchJointSessionJob::MeshRestart");
                    return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
                }
                if (m_reserveda8[0] == 0)
                {
                    m_time178 = deadlineAfter(cRetryJoinIntervalMsec);
                    if (joinResult == cResultJoinMesh6439 || joinResult == cResultJoinMesh643F ||
                        joinResult == cResultJoinMesh6442 || joinResult == cResultJoinMesh6445 ||
                        joinResult == cResultJoinMesh6443 || joinResult == cResultJoinMesh6444 ||
                        joinResult == cResultJoinMesh6467 || joinResult == cResultJoinMesh646A ||
                        joinResult == cResultJoinMesh6468 || joinResult == cResultJoinMesh6469 ||
                        joinResult == cResultJoinMesh644A)
                    {
                        countRetry(common::g_sessionBeginMonitoringContent.m_retryReasonACount);
                    }
                    else if (m_reserveda8[1] != 0)
                    {
                        countRetry(common::g_sessionBeginMonitoringContent.m_retryReasonBCount);
                    }
                }
                else
                {
                    m_time178 = deadlineAfter(cRetryJoinIntervalHostMsec);
                    countRetry(common::g_sessionBeginMonitoringContent.m_sessionInfoRetryCount);
                }
                setStep(*this, &NexMatchJointSessionJob::WaitStartRetryJoinMesh,
                        "NexMatchJointSessionJob::WaitStartRetryJoinMesh");
                return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
            }
        }

        if (m_fielda2 == 0)
        {
            if (joinResult == cResultJoinMesh6C05)
                m_fieldc8 = cFailurePointJoinMesh6C05;
            else if (joinResult == cResultInvalidState)
                m_fieldc8 = cFailurePointInvalidState;
            else if (joinResult == cResultJoinMesh6439)
                m_fieldc8 = cFailurePointJoinMesh6439;
            else if (joinResult == cResultJoinMesh643F)
                m_fieldc8 = cFailurePointJoinMesh643F;
            else if (joinResult == cResultJoinMesh6442 || joinResult == cResultJoinMesh6467)
                m_fieldc8 = cFailurePointJoinMesh6442;
            else if (joinResult == cResultJoinMesh6445 || joinResult == cResultJoinMesh646A)
                m_fieldc8 = cFailurePointJoinMesh6445;
            else if (joinResult == cResultJoinMesh6443 || joinResult == cResultJoinMesh6468)
                m_fieldc8 = cFailurePointJoinMesh6443;
            else if (joinResult == cResultJoinMesh6444 || joinResult == cResultJoinMesh6469)
                m_fieldc8 = cFailurePointJoinMesh6444;
            else if (joinResult == cResultJoinMesh644A)
                m_fieldc8 = cFailurePointJoinMesh644A;
            else if (joinResult == cResultJoinMesh6838)
                m_fieldc8 = cFailurePointJoinMesh6838;
            else if (joinResult == cResultJoinMesh6C3D)
                m_fieldc8 = cFailurePointJoinMesh6C3D;
            else if (joinResult == cResultJoinMesh6846)
                m_fieldc8 = cFailurePointJoinMesh6846;
            else if (joinResult == cResultJoinMesh6847)
                m_fieldc8 = cFailurePointJoinMesh6847;
            else if (joinResult == cResultJoinMesh6848)
                m_fieldc8 = cFailurePointJoinMesh6848;
            else if (joinResult == cResultJoinMesh6849)
                m_fieldc8 = cFailurePointJoinMesh6849;
            else if (joinResult == cResultJoinMesh10C3E)
                m_fieldc8 = cFailurePointJoinMesh10C3E;
            else if (joinResult == cResultJoinMesh10C25)
                m_fieldc8 = cFailurePointJoinMesh10C25;
            else
                m_fieldc8 = cFailurePointJoinMeshOther;
        }
        else
        {
            m_fieldc8 = cFailurePointDisconnectDetected;
        }
        if (m_fielda2 != 0)
            joinResult = m_fielda4;
        m_failureResult = joinResult;
        unregisterBufferSearchCriteria();
        RecordMonitoringDataWhenProcessFailure();
        setStep(*this, &NexMatchJointSessionJob::StartLeaveMesh,
                "NexMatchJointSessionJob::StartLeaveMesh");
        return common::Job::MakeExecuteResult(common::Job::cExecuteKindNextStep);
    }

    if (IsMigrateSessionToJointSession())
        m_fielda3 = 1;
    session->m_stationIdStatusTable->SetMigrationCompleteStatus(session->m_localStationId, true);
    session::g_piaSession->m_disconnectStatus = 1;
    setStep(*this, &NexMatchJointSessionJob::WaitCompanionStation,
            "NexMatchJointSessionJob::WaitCompanionStation");
    m_jointSessionState = cJointSessionStateWaitCompanionStation;
    return common::Job::MakeExecuteResult(common::Job::cExecuteKindContinue);
}


}}}
