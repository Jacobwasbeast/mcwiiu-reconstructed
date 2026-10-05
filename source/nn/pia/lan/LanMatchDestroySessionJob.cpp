#include "nn/pia/lan/LanMatchDestroySessionJob.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/common/Job.hpp"

namespace nn { namespace pia { namespace lan {

LanMatchDestroySessionJob::LanMatchDestroySessionJob()
    : session::DestroySessionJob()
{
}

LanMatchDestroySessionJob::~LanMatchDestroySessionJob()
{
}

LanMatchDestroySessionJob::ExecuteResult LanMatchDestroySessionJob::UnregisterCurrentMatchmakeSession()
{
    const unsigned int index = session::g_piaSession->m_currentSessionIndex;
    session::IMatchmakeSession* matchmake = session::g_piaSession->m_matchmakeSession[index];
    const Result result = matchmake->UnregisterMatchmakeSession(&m_callContext,
                                                                 session::g_piaSession->m_sessionId[index]);
    if (result != 0)
    {
        setStep(&session::DestroySessionJob::SendMonitoringData,
                "DestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    setStep(static_cast<common::Job::ExecuteResult (session::DestroySessionJob::*)()>(&LanMatchDestroySessionJob::WaitUnregisterCurrentMatchmakeSession),
            "LanMatchDestroySessionJob::WaitUnregisterCurrentMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchDestroySessionJob::ExecuteResult LanMatchDestroySessionJob::WaitUnregisterCurrentMatchmakeSession()
{
    const unsigned int index = session::g_piaSession->m_currentSessionIndex;
    session::IMatchmakeSession* matchmake = session::g_piaSession->m_matchmakeSession[index];
    if (!matchmake->WaitUnregisterMatchmakeSession())
        return MakeExecuteResult(cExecuteKindNextStep);
    session::g_piaSession->m_sessionId[index] = 0;
    setStep(&session::DestroySessionJob::SendMonitoringData,
            "DestroySessionJob::SendMonitoringData");
    return MakeExecuteResult(cExecuteKindContinue);
}

void LanMatchDestroySessionJob::Cleanup()
{
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
    }
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
}

void LanMatchDestroySessionJob::Trace(unsigned long long) const
{
}

Result LanMatchDestroySessionJob::StartupImpl()
{
    return 0;
}

void LanMatchDestroySessionJob::SetNextStepAtWaitDestroyMesh()
{
    setStep(&session::DestroySessionJob::MeshCleanup,
            "DestroySessionJob::MeshCleanup");
}

void LanMatchDestroySessionJob::SetNextStepAtMeshCleanup()
{
    setStep(static_cast<common::Job::ExecuteResult (session::DestroySessionJob::*)()>(&LanMatchDestroySessionJob::UnregisterCurrentMatchmakeSession),
            "LanMatchDestroySessionJob::UnregisterCurrentMatchmakeSession");
}

}}}
