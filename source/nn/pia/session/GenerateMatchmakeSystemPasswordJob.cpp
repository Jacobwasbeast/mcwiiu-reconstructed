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

    const Result operationResult = GenerateMatchmakeSystemPassword();
    if (operationResult != 0)
        return operationResult;

    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_sessionId = sessionId;
    Reset(true);
    return 0;
}

GenerateMatchmakeSystemPasswordJob::ExecuteResult GenerateMatchmakeSystemPasswordJob::CompleteProcess()
{
    m_callerContext->SignalSuccess(0);
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

GenerateMatchmakeSystemPasswordJob::ExecuteResult GenerateMatchmakeSystemPasswordJob::FailureProcess()
{
    if (m_callerContext->m_result == static_cast<Result>(0x8c56))
        g_piaSession->ChangeStatusToDisconnectedSession();
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
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
