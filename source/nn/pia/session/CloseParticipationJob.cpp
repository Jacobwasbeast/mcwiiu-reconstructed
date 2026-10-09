#include "nn/pia/session/CloseParticipationJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
unsigned int SessionFailureResult(unsigned int status)
{
    switch (status)
    {
    case Session::cStatusDisconnectedNetwork:
        return Session::cResultStatusDisconnectedNetwork;
    case Session::cStatusUnknown6:
        return Session::cResultStatusUnknown6;
    case Session::cStatusUnknown7:
        return Session::cResultStatusUnknown7;
    case Session::cStatusUnknown8:
        return Session::cResultStatusUnknown8;
    case Session::cStatusDisconnectedSession:
        return Session::cResultStatusDisconnectedSession;
    default:
        return 0;
    }
}

bool IsP2pStable(unsigned int sessionId)
{
    (void)sessionId;
    return g_piaMesh != 0 && g_piaMesh->GetMeshState() == Mesh::cStateStarted;
}

unsigned int ElapsedMilliseconds(const common::Time& start)
{
    common::Time now;
    now.SetNow();
    return static_cast<unsigned int>(
        (now.m_tick - start.m_tick) / common::TimeSpan::GetTicksPerMSec().m_tick);
}
}

CloseParticipationJob::CloseParticipationJob()
    : common::StepSequenceJob(), m_callerContext(0), m_callContext(),
      m_sessionId(0), m_matchmakeSession(0), m_startTime()
{
}

CloseParticipationJob::~CloseParticipationJob()
{
}

Result CloseParticipationJob::Startup(common::CallContext* callerContext,
                                      unsigned int sessionId,
                                      IMatchmakeSession* matchmakeSession)
{
    if (callerContext == 0 || matchmakeSession == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    if (!matchmakeSession->IsMatchmakeOwner())
        return static_cast<Result>(transport::cResultInvalidState);

    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_matchmakeSession = matchmakeSession;
    m_sessionId = sessionId;
    Reset(true);
    m_currentStep.m_function = static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(&CloseParticipationJob::CloseParticipation);
    m_currentStep.m_name = "CloseParticipationJob::CloseParticipation";
    return 0;
}

CloseParticipationJob::ExecuteResult CloseParticipationJob::CloseParticipation()
{
    const unsigned int failure = SessionFailureResult(g_piaSession->GetStatus());
    if (failure != 0)
    {
        m_callerContext->SignalFailure(static_cast<Result>(failure));
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalCancel();
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }

    const Result result = m_matchmakeSession->CloseParticipate(&m_callContext, m_sessionId);
    if (result == 0)
    {
        m_currentStep.m_function = static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(&CloseParticipationJob::WaitCloseParticipation);
        m_currentStep.m_name = "CloseParticipationJob::WaitCloseParticipation";
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (result == static_cast<Result>(transport::cResultTransportUnavailable))
        return MakeExecuteResult(cExecuteKindNextStep);
    m_callerContext->SignalFailure(result);
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

CloseParticipationJob::ExecuteResult CloseParticipationJob::WaitCloseParticipation()
{
    const unsigned int failure = SessionFailureResult(g_piaSession->GetStatus());
    if (failure != 0)
    {
        m_callerContext->SignalFailure(static_cast<Result>(failure));
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (!m_matchmakeSession->WaitCloseParticipate())
        return MakeExecuteResult(cExecuteKindNextStep);

    if (m_callContext.m_state == common::CallContext::cStateFailure)
    {
        if (m_callContext.m_result == static_cast<Result>(transport::cResultInvalidState))
        {
            g_piaSession->ChangeStatusToDisconnectedNetwork();
            m_callerContext->SignalFailure(static_cast<Result>(Session::cResultStatusDisconnectedSession));
        }
        else
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
        }
    }
    if (m_callContext.m_state != common::CallContext::cStateSuccess)
    {
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    m_currentStep.m_function = static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(&CloseParticipationJob::WaitP2pStable);
    m_currentStep.m_name = "CloseParticipationJob::WaitP2pStable";
    m_startTime.SetNow();
    return MakeExecuteResult(cExecuteKindContinue);
}

CloseParticipationJob::ExecuteResult CloseParticipationJob::WaitP2pStable()
{
    const unsigned int failure = SessionFailureResult(g_piaSession->GetStatus());
    if (failure != 0)
    {
        m_callerContext->SignalFailure(static_cast<Result>(failure));
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }

    const unsigned int elapsed = ElapsedMilliseconds(m_startTime);
    if (elapsed > 1000 && IsP2pStable(m_sessionId))
    {
        if (g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex] == m_sessionId)
            g_piaSession->SetCurrentSessionOpenStatus(
                Session::cSessionOpenStatusParticipationClosed);
        else
            g_piaSession->SetBufferSessionOpenStatus(
                Session::cSessionOpenStatusParticipationClosed);
        m_callerContext->SignalSuccess(0);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    const unsigned int finalElapsed = ElapsedMilliseconds(m_startTime);
    if (finalElapsed > 15000)
    {
        m_callerContext->SignalFailure(static_cast<Result>(0x2c18));
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void CloseParticipationJob::Cleanup()
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

void CloseParticipationJob::Trace(unsigned long long) const
{
}

}}}
