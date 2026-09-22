#include "nn/pia/session/GenerateMatchmakeSystemPasswordJob.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn { namespace pia { namespace session {

GenerateMatchmakeSystemPasswordJob::GenerateMatchmakeSystemPasswordJob()
    : common::StepSequenceJob(), m_sessionId(0), m_callerContext(0), m_callContext()
{
}

GenerateMatchmakeSystemPasswordJob::~GenerateMatchmakeSystemPasswordJob()
{
}

Result GenerateMatchmakeSystemPasswordJob::Startup(common::CallContext* callerContext,
                                                    unsigned int sessionId,
                                                    IMatchmakeSession* session)
{
    if (callerContext == 0 || session == 0)
        return 0x10c07;
    if (!session->IsMatchmakeOwner())
        return 0x10c08;

    common::CallContext operationContext;
    const Result operationResult =
        session->GenerateMatchmakeSystemPassword(&operationContext, sessionId);
    if (operationResult != 0)
        return operationResult;

    m_callerContext = callerContext;
    m_sessionId = sessionId;
    callerContext->InitiateCall();
    Reset(true);
    return 0;
}

unsigned int GenerateMatchmakeSystemPasswordJob::CompleteProcess()
{
    m_callerContext->SignalSuccess(0);
    m_callerContext = 0;
    return common::Job::cExecuteResultFinished;
}

unsigned int GenerateMatchmakeSystemPasswordJob::FailureProcess()
{
    if (m_callerContext->m_result == static_cast<Result>(0x8c56))
        g_piaSession->ChangeStatusToDisconnectedSession();
    m_callerContext = 0;
    return common::Job::cExecuteResultFinished;
}

void GenerateMatchmakeSystemPasswordJob::Cleanup()
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
    m_sessionId = 0;
}

void GenerateMatchmakeSystemPasswordJob::Trace(unsigned long long) const
{
}

}}}
