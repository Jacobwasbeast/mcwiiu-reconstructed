#include "nn/pia/session/DestroySessionJob.hpp"

#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"

namespace nn { namespace pia { namespace session {

DestroySessionJob::DestroySessionJob()
    : common::StepSequenceJob(), m_callerContext(0), m_callContext(), m_startTick(0), m_endTick(0),
      m_isSingleStation(false), m_padding()
{
}

DestroySessionJob::~DestroySessionJob()
{
}

void DestroySessionJob::setStep(common::Job::ExecuteResult (DestroySessionJob::*function)(), const char* name)
{
    m_currentStep.m_function = static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function);
    m_currentStep.m_name = name;
}

Result DestroySessionJob::Startup(common::CallContext* callerContext)
{
    if (m_callerContext != 0 || callerContext == 0 || g_piaMesh == 0 || g_piaSession == 0)
        return static_cast<Result>(0x10c08);
    if (g_piaMesh->IsHostMigrationEnabled())
        return static_cast<Result>(0x10c08);
    m_callerContext = callerContext;
    m_callContext.Reset();
    callerContext->InitiateCall();
    if (common::g_piaJobScheduler != 0)
    {
        const unsigned long long now = common::g_piaJobScheduler->m_dispatchTime.m_tick;
        m_startTick = static_cast<int>(now >> 32);
        m_endTick = static_cast<int>(now);
    }
    m_isSingleStation = false;
    if (g_piaSession->m_jointSessionJob != 0)
    {
        g_piaSession->m_jointSessionJob->m_fielda2 = 1;
        g_piaSession->m_jointSessionJob->m_fielda4 = 0x6c05;
    }
    setStep(&DestroySessionJob::WaitForcedTerminatingOfJointSessionJob,
             "DestroySessionJob::WaitForcedTerminatingOfJointSessionJob");
    Reset(true);
    return 0;
}

DestroySessionJob::ExecuteResult DestroySessionJob::WaitForcedTerminatingOfJointSessionJob()
{
    if (g_piaSession != 0 && g_piaSession->GetStatus() == 5)
        return SendMonitoringData();
    if (g_piaSession != 0 && g_piaSession->m_jointSessionJob != 0 &&
        g_piaSession->m_jointSessionJob->IsRunning())
    {
        m_isSingleStation = true;
        setStep(&DestroySessionJob::DestroyMesh, "DestroySessionJob::DestroyMesh");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    setStep(&DestroySessionJob::DestroyMesh, "DestroySessionJob::DestroyMesh");
    return MakeExecuteResult(cExecuteKindNextStep);
}

DestroySessionJob::ExecuteResult DestroySessionJob::DestroyMesh()
{
    if (g_piaSession != 0 && g_piaSession->GetStatus() == 5)
        return SendMonitoringData();
    g_piaMesh->SetSkipReport(true);
    if (m_isSingleStation)
    {
        setStep(&DestroySessionJob::MeshCleanup, "DestroySessionJob::MeshCleanup");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    Result result = 0;
    result = g_piaMesh->DestroyMesh(&m_callContext);
    if (result != 0)
        return SendMonitoringData();
    setStep(&DestroySessionJob::WaitDestroyMesh, "DestroySessionJob::WaitDestroyMesh");
    return MakeExecuteResult(cExecuteKindWait, 5);
}

DestroySessionJob::ExecuteResult DestroySessionJob::WaitDestroyMesh()
{
    if (g_piaSession != 0 && g_piaSession->GetStatus() == 5)
        return SendMonitoringData();
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return MakeExecuteResult(cExecuteKindWait, 5);
    if (m_callContext.m_state == common::CallContext::cStateFailure)
    {
        return SendMonitoringData();
    }
    setStep(&DestroySessionJob::MeshCleanup, "DestroySessionJob::MeshCleanup");
    return MakeExecuteResult(cExecuteKindNextStep);
}

DestroySessionJob::ExecuteResult DestroySessionJob::MeshCleanup()
{
    g_piaMesh->ResetInternalAsyncState();
    g_piaMesh->SetSkipReport(false);
    g_piaMesh->SendReport(1);
    return SendMonitoringData();
}

void DestroySessionJob::CalcProcessingTimeOfLeave() const
{
    if (common::g_piaJobScheduler == 0)
        return;
    const unsigned long long start =
        (static_cast<unsigned long long>(static_cast<unsigned int>(m_startTick)) << 32) |
        static_cast<unsigned int>(m_endTick);
    const unsigned long long elapsed = common::g_piaJobScheduler->m_dispatchTime.m_tick - start;
    const long long ticksPerMSec = common::TimeSpan::GetTicksPerMSec().m_tick;
    if (ticksPerMSec != 0)
        const_cast<DestroySessionJob*>(this)->m_startTick =
            static_cast<int>(elapsed / static_cast<unsigned long long>(ticksPerMSec));
}

DestroySessionJob::ExecuteResult DestroySessionJob::SendMonitoringData()
{
    CalcProcessingTimeOfLeave();
    g_piaMesh->SetSkipReport(false);
    g_piaMesh->SendReport(1);
    setStep(&DestroySessionJob::CompleteProcess, "DestroySessionJob::CompleteProcess");
    return MakeExecuteResult(cExecuteKindWait, 0x32);
}

DestroySessionJob::ExecuteResult DestroySessionJob::CompleteProcess()
{
    if (g_piaMesh != 0)
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

void DestroySessionJob::Cleanup()
{
    if (m_callerContext != 0 && m_callerContext->m_state == common::CallContext::cStateInProgress)
        m_callerContext->SignalCancel();
    m_callerContext = 0;
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
}

void DestroySessionJob::Trace(unsigned long long) const {}

}}}
