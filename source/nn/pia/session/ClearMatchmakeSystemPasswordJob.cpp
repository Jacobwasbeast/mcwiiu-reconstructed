#include "nn/pia/session/ClearMatchmakeSystemPasswordJob.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn { namespace pia { namespace session {

ClearMatchmakeSystemPasswordJob::ClearMatchmakeSystemPasswordJob()
    : common::StepSequenceJob(), m_sessionId(0), m_callerContext(0), m_callContext()
{
}

ClearMatchmakeSystemPasswordJob::~ClearMatchmakeSystemPasswordJob()
{
}

Result ClearMatchmakeSystemPasswordJob::Startup(common::CallContext* callerContext,
                                                unsigned int sessionId,
                                                IMatchmakeSession* matchmakeSession)
{
    if (callerContext == 0 || matchmakeSession == 0)
        return static_cast<Result>(0x10c07);
    if (!matchmakeSession->IsMatchmakeOwner())
        return static_cast<Result>(0x10c08);

    const Result operationResult = ClearMatchmakeSystemPassword();
    if (operationResult != 0)
        return operationResult;

    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_sessionId = sessionId;
    Reset(true);
    return 0;
}

ClearMatchmakeSystemPasswordJob::ExecuteResult ClearMatchmakeSystemPasswordJob::CompleteProcess()
{
    m_callerContext->SignalSuccess(0);
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

ClearMatchmakeSystemPasswordJob::ExecuteResult ClearMatchmakeSystemPasswordJob::FailureProcess()
{
    if (m_callerContext->m_result == static_cast<Result>(0x8c56))
        g_piaSession->ChangeStatusToDisconnectedSession();
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

void ClearMatchmakeSystemPasswordJob::Cleanup()
{
    if (m_callerContext != 0 &&
        m_callerContext->m_state == common::CallContext::cStateInProgress)
        m_callerContext->SignalCancel();
    m_callerContext = 0;
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
    m_sessionId = 0;
}

void ClearMatchmakeSystemPasswordJob::Trace(unsigned long long) const
{
}

}}}
