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

uint32_t LanMatchDestroySessionJob::UnregisterCurrentMatchmakeSession()
{
    const unsigned int index = session::g_piaSession->m_currentSessionIndex;
    session::IMatchmakeSession* matchmake = session::g_piaSession->m_matchmakeSession[index];
    const Result result = matchmake->UnregisterMatchmakeSession(&m_callContext,
                                                                 session::g_piaSession->m_sessionId[index]);
    if (result != 0)
    {
        setStep(&session::DestroySessionJob::SendMonitoringData,
                "DestroySessionJob::SendMonitoringData");
        return static_cast<unsigned int>(session::g_piaSession->m_sessionId[index] & 0xff0000u);
    }
    setStep(static_cast<unsigned int (session::DestroySessionJob::*)()>(&LanMatchDestroySessionJob::WaitUnregisterCurrentMatchmakeSession),
            "LanMatchDestroySessionJob::WaitUnregisterCurrentMatchmakeSession");
    return (session::g_piaSession->m_sessionId[index] & 0xff0000u) | common::Job::cExecuteResultNextStep;
}

uint32_t LanMatchDestroySessionJob::WaitUnregisterCurrentMatchmakeSession()
{
    const unsigned int index = session::g_piaSession->m_currentSessionIndex;
    session::IMatchmakeSession* matchmake = session::g_piaSession->m_matchmakeSession[index];
    if (!matchmake->WaitUnregisterMatchmakeSession())
        return common::Job::cExecuteResultNextStep;
    session::g_piaSession->m_sessionId[index] = 0;
    setStep(&session::DestroySessionJob::SendMonitoringData,
            "DestroySessionJob::SendMonitoringData");
    return 0;
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

void LanMatchDestroySessionJob::StartupImpl(unsigned int* result)
{
    if (result != 0)
        *result = 0;
}

void LanMatchDestroySessionJob::SetNextStepAtWaitDestroyMesh()
{
    setStep(&session::DestroySessionJob::MeshCleanup,
            "DestroySessionJob::MeshCleanup");
}

void LanMatchDestroySessionJob::SetNextStepAtMeshCleanup()
{
    setStep(static_cast<unsigned int (session::DestroySessionJob::*)()>(&LanMatchDestroySessionJob::UnregisterCurrentMatchmakeSession),
            "LanMatchDestroySessionJob::UnregisterCurrentMatchmakeSession");
}

}}}
