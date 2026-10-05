#include "nn/pia/session/LeaveSessionJob.hpp"

#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/JointSessionJob.hpp"

namespace nn { namespace pia { namespace session {
extern Session* g_piaSession;

namespace
{
static bool hasSessionFailed(Session* session)
{
    const unsigned int status = session != 0 ? session->GetStatus() : 0;
    return status == 5 || status == 6 || status == 7 || status == 8;
}
}

LeaveSessionJob::LeaveSessionJob()
    : common::StepSequenceJob(), m_callerContext(0), m_callContext(),
      m_startTick(0), m_endTick(0), m_isSingleStation(false), m_padding(),
      m_field64(0), m_migrationStartTick(0), m_migrationEndTick(0)
{
}

LeaveSessionJob::~LeaveSessionJob()
{
}

void LeaveSessionJob::setStep(common::Job::ExecuteResult (LeaveSessionJob::*function)(), const char* name)
{
    m_currentStep.m_function = static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function);
    m_currentStep.m_name = name;
}

Result LeaveSessionJob::Startup(common::CallContext* callerContext)
{
    if (m_callerContext != 0 || callerContext == 0 || g_piaMesh == 0 || g_piaSession == 0)
        return static_cast<Result>(0x10c08);
    if (g_piaMesh->IsHost() && !g_piaMesh->IsHostMigrationEnabled())
        return static_cast<Result>(0x10c08);
    m_callerContext = callerContext;
    m_callContext.Reset();
    callerContext->InitiateCall();
    m_isSingleStation = false;
    if (common::g_piaJobScheduler != 0)
    {
        m_startTick = static_cast<int>(common::g_piaJobScheduler->m_dispatchTime.m_tick >> 32);
        m_endTick = static_cast<int>(common::g_piaJobScheduler->m_dispatchTime.m_tick);
    }
    g_piaMesh->SetSkipReport(true);
    if (g_piaSession->m_jointSessionJob != 0)
    {
        g_piaSession->m_jointSessionJob->m_fielda2 = 1;
        g_piaSession->m_jointSessionJob->m_fielda4 = 0x6c05;
    }
    setStep(&LeaveSessionJob::WaitForcedTerminatingOfJointSessionJob,
             "LeaveSessionJob::WaitForcedTerminatingOfJointSessionJob");
    Reset(true);
    return 0;
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::WaitForcedTerminatingOfJointSessionJob()
{
    if (hasSessionFailed(g_piaSession))
        return SendMonitoringData();
    if (g_piaSession != 0 && g_piaSession->m_jointSessionJob != 0 &&
        g_piaSession->m_jointSessionJob->IsRunning())
        return MakeExecuteResult(cExecuteKindWait, 5);
    setStep(&LeaveSessionJob::LeaveMesh, "LeaveSessionJob::LeaveMesh");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::LeaveMesh()
{
    if (hasSessionFailed(g_piaSession))
        return SendMonitoringData();
    Result result = 0;
    const bool migrate = g_piaMesh->IsHost() && g_piaMesh->IsHostMigrationEnabled();
    if (migrate)
    {
        m_isSingleStation = g_piaMesh->GetStationNum() == 1;
        result = g_piaMesh->LeaveMeshWithHostMigration(&m_callContext);
        setStep(&LeaveSessionJob::WaitLeaveMeshWithHostMigration,
                 "LeaveSessionJob::WaitLeaveMeshWithHostMigration");
    }
    else
    {
        result = g_piaMesh->LeaveMesh(&m_callContext);
        setStep(&LeaveSessionJob::WaitLeaveMesh, "LeaveSessionJob::WaitLeaveMesh");
    }
    if (result != 0)
        return SendMonitoringData();
    return MakeExecuteResult(cExecuteKindWait, 5);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::WaitLeaveMesh()
{
    if (hasSessionFailed(g_piaSession))
        return SendMonitoringData();
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return MakeExecuteResult(cExecuteKindWait, 5);
    if (m_callContext.m_state == common::CallContext::cStateFailure)
    {
        return SendMonitoringData();
    }
    SetNextStepAtWaitLeaveMesh();
    return MakeExecuteResult(cExecuteKindNextStep);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::WaitLeaveMeshWithHostMigration()
{
    if (hasSessionFailed(g_piaSession))
        return SendMonitoringData();
    if (m_isSingleStation)
    {
        SetNextStepAtWaitHostMigratedTimeout();
        return MakeExecuteResult(cExecuteKindWait, 5);
    }
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return MakeExecuteResult(cExecuteKindWait, 5);
    if (m_callContext.m_state == common::CallContext::cStateFailure)
    {
        return SendMonitoringData();
    }
    SetNextStepAtWaitHostMigratedTimeout();
    return MakeExecuteResult(cExecuteKindNextStep);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::WaitHostMigrated()
{
    if (hasSessionFailed(g_piaSession))
        return SendMonitoringData();
    if (common::g_piaJobScheduler != 0 && GetWaitHostMigrationTimeout() != 0)
    {
        const unsigned long long now = common::g_piaJobScheduler->m_dispatchTime.m_tick;
        const unsigned long long start =
            (static_cast<unsigned long long>(static_cast<unsigned int>(m_migrationStartTick)) << 32) |
            static_cast<unsigned int>(m_migrationEndTick);
        if (now - start < GetWaitHostMigrationTimeout())
            return MakeExecuteResult(cExecuteKindWait, 5);
    }
    SetNextStepAtWaitLeaveCurrentMatchmakeSession();
    return MakeExecuteResult(cExecuteKindNextStep);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::LeaveCurrentMatchmakeSession()
{
    if (g_piaSession == 0)
        return SendMonitoringData();
    const unsigned int index = g_piaSession->m_currentSessionIndex;
    IMatchmakeSession* matchmake = g_piaSession->m_matchmakeSession[index];
    if (matchmake == 0)
        return SendMonitoringData();
    const Result result = matchmake->LeaveMatchmakeSession(
        &m_callContext, g_piaSession->m_sessionId[index]);
    if (result != 0)
    {
        m_callContext.SetResult(result);
        SetNextStepAtLeaveCurrentMatchmakeSessionFailure();
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    SetNextStepAtWaitLeaveCurrentMatchmakeSession();
    return MakeExecuteResult(cExecuteKindWait, 5);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::WaitLeaveCurrentMatchmakeSession()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return MakeExecuteResult(cExecuteKindWait, 5);
    if (m_callContext.m_state == common::CallContext::cStateFailure ||
        m_callContext.m_state == common::CallContext::cStateCancelled)
    {
        SetNextStepAtLeaveCurrentMatchmakeSessionFailure();
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    const unsigned int index = g_piaSession->m_currentSessionIndex;
    IMatchmakeSession* matchmake = g_piaSession->m_matchmakeSession[index];
    if (matchmake == 0 || !matchmake->WaitLeaveMatchmakeSession())
        return MakeExecuteResult(cExecuteKindWait, 5);
    const Result result = m_callContext.m_result;
    if (result != 0)
        SetNextStepAtLeaveCurrentMatchmakeSessionFailure();
    else
    {
        matchmake->Cleanup();
        g_piaSession->m_sessionId[index] = 0;
        SetNextStepAtWaitLeaveCurrentMatchmakeSession();
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void LeaveSessionJob::CalcProcessingTimeOfLeave() const
{
    if (common::g_piaJobScheduler == 0)
        return;
    const unsigned long long start =
        (static_cast<unsigned long long>(static_cast<unsigned int>(m_startTick)) << 32) |
        static_cast<unsigned int>(m_endTick);
    const unsigned long long elapsed = common::g_piaJobScheduler->m_dispatchTime.m_tick - start;
    const long long ticksPerMSec = common::TimeSpan::GetTicksPerMSec().m_tick;
    if (ticksPerMSec != 0)
        const_cast<LeaveSessionJob*>(this)->m_field64 =
            static_cast<unsigned int>(elapsed / static_cast<unsigned long long>(ticksPerMSec));
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::SendMonitoringData()
{
    if (g_piaMesh != 0)
    {
        g_piaMesh->SetSkipReport(false);
        g_piaMesh->SendReport(1);
    }
    setStep(&LeaveSessionJob::CompleteProcess, "LeaveSessionJob::CompleteProcess");
    return MakeExecuteResult(cExecuteKindWait, 0x32);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::MeshCleanup()
{
    if (g_piaMesh != 0)
        g_piaMesh->ResetInternalAsyncState();
    SetNextStepAtMeshCleanup();
    return MakeExecuteResult(cExecuteKindNextStep);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::CompleteProcess()
{
    if (g_piaSession != 0)
        g_piaMesh->CleanupStatus();
    if (m_callerContext != 0)
    {
        if (m_callContext.m_result != 0)
            m_callerContext->SignalFailure(m_callContext.m_result);
        else
            m_callerContext->SignalSuccess(0);
    }
    m_callerContext = 0;
    if (g_piaMesh != 0)
        g_piaMesh->SetSkipReport(false);
    return MakeExecuteResult(cExecuteKindFinished);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::LeaveBufferMatchmakeSession()
{
    if (g_piaSession == 0)
        return SendMonitoringData();
    const unsigned int index = g_piaSession->m_currentSessionIndex == 0 ? 1 : 0;
    IMatchmakeSession* matchmake = g_piaSession->m_matchmakeSession[index];
    if (matchmake == 0)
        return SendMonitoringData();
    const Result result = matchmake->LeaveMatchmakeSession(
        &m_callContext, g_piaSession->m_sessionId[index]);
    if (result != 0)
        return SendMonitoringData();
    SetNextStepAtWaitLeaveBufferMatchmakeSession();
    return MakeExecuteResult(cExecuteKindWait, 5);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::WaitLeaveBufferMatchmakeSession()
{
    const unsigned int index = g_piaSession->m_currentSessionIndex == 0 ? 1 : 0;
    IMatchmakeSession* matchmake = g_piaSession->m_matchmakeSession[index];
    if (matchmake == 0 || !matchmake->WaitLeaveMatchmakeSession())
        return MakeExecuteResult(cExecuteKindWait, 5);
    matchmake->Cleanup();
    g_piaSession->m_sessionId[index] = 0;
    SetNextStepAtWaitLeaveBufferMatchmakeSession();
    return MakeExecuteResult(cExecuteKindNextStep);
}

void LeaveSessionJob::SetNextStepAtWaitForcedTerminatingOfJointSessionJob() { setStep(&LeaveSessionJob::LeaveMesh, "LeaveSessionJob::LeaveMesh"); }
void LeaveSessionJob::SetNextStepAtWaitLeaveMesh() { setStep(&LeaveSessionJob::MeshCleanup, "LeaveSessionJob::MeshCleanup"); }
void LeaveSessionJob::SetNextStepAtMeshCleanup() { setStep(&LeaveSessionJob::LeaveCurrentMatchmakeSession, "LeaveSessionJob::LeaveCurrentMatchmakeSession"); }
void LeaveSessionJob::SetNextStepAtWaitLeaveCurrentMatchmakeSession() { setStep(&LeaveSessionJob::SendMonitoringData, "LeaveSessionJob::SendMonitoringData"); }
void LeaveSessionJob::SetNextStepAtLeaveCurrentMatchmakeSessionFailure() { setStep(&LeaveSessionJob::SendMonitoringData, "LeaveSessionJob::SendMonitoringData"); }
void LeaveSessionJob::SetNextStepAtWaitHostMigratedTimeout()
{
    if (common::g_piaJobScheduler != 0)
    {
        const unsigned long long now = common::g_piaJobScheduler->m_dispatchTime.m_tick;
        m_migrationStartTick = static_cast<int>(now >> 32);
        m_migrationEndTick = static_cast<int>(now);
    }
    setStep(&LeaveSessionJob::WaitHostMigrated, "LeaveSessionJob::WaitHostMigrated");
}
void LeaveSessionJob::SetNextStepAtWaitLeaveBufferMatchmakeSession() { setStep(&LeaveSessionJob::LeaveCurrentMatchmakeSession, "LeaveSessionJob::LeaveCurrentMatchmakeSession"); }

void LeaveSessionJob::Cleanup()
{
    CleanupImpl();
    if (m_callerContext != 0 && m_callerContext->m_state == common::CallContext::cStateInProgress)
        m_callerContext->SignalCancel();
    m_callerContext = 0;
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
    m_isSingleStation = false;
}

void LeaveSessionJob::Trace(unsigned long long) const {}
unsigned int LeaveSessionJob::GetWaitHostMigrationTimeout() const { return 0; }
void LeaveSessionJob::ChangeStatusToDisconnectedNetwork() { if (g_piaSession != 0) g_piaSession->ChangeStatusToDisconnectedNetwork(); }
void LeaveSessionJob::CleanupImpl() {}

}}}
