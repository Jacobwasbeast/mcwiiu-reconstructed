#include "nn/pia/session/CloseParticipationJob.hpp"

#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
const unsigned int kRun = 0x1000000;
const unsigned int kWait = 0x5000000;
const unsigned int kStep = 0x4d0000;
const unsigned int kCloseWait = 0x54d0000;
const unsigned int kDone = 0x14d0000;

unsigned int SessionFailureResult(unsigned int status)
{
    if (status == 5) return 0xc406;
    if (status == 6) return 0xa471;
    if (status == 7) return 0xc420;
    if (status == 8) return 0xc421;
    return status == 4 ? 0x8c56 : 0;
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
        return static_cast<Result>(0x10c07);
    if (!matchmakeSession->IsMatchmakeOwner())
        return static_cast<Result>(0x10c08);

    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_matchmakeSession = matchmakeSession;
    m_sessionId = sessionId;
    Reset(true);
    m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(&CloseParticipationJob::CloseParticipation);
    m_currentStep.m_name = "CloseParticipationJob::CloseParticipation";
    return 0;
}

unsigned int CloseParticipationJob::CloseParticipation()
{
    const unsigned int failure = SessionFailureResult(g_piaSession->GetStatus());
    if (failure != 0)
    {
        m_callerContext->SignalFailure(static_cast<Result>(failure));
        m_callerContext = 0;
        return kDone;
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalCancel();
        m_callerContext = 0;
        return kDone;
    }

    const Result result = m_matchmakeSession->CloseParticipate(&m_callContext, m_sessionId);
    if (result == 0)
    {
        m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(&CloseParticipationJob::WaitCloseParticipation);
        m_currentStep.m_name = "CloseParticipationJob::WaitCloseParticipation";
        return kCloseWait;
    }
    if (result == static_cast<Result>(0x4c11))
        return kCloseWait;
    m_callerContext->SignalFailure(result);
    m_callerContext = 0;
    return kDone;
}

unsigned int CloseParticipationJob::WaitCloseParticipation()
{
    const unsigned int failure = SessionFailureResult(g_piaSession->GetStatus());
    if (failure != 0)
    {
        m_callerContext->SignalFailure(static_cast<Result>(failure));
        m_callerContext = 0;
        return kDone;
    }
    if (!m_matchmakeSession->WaitCloseParticipate())
        return kWait;

    if (m_callContext.m_state == common::CallContext::cStateFailure)
    {
        if (m_callContext.m_result == static_cast<Result>(0x10c08))
        {
            g_piaSession->ChangeStatusToDisconnectedNetwork();
            m_callerContext->SignalFailure(static_cast<Result>(0x8c56));
        }
        else
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
        }
    }
    if (m_callContext.m_state != common::CallContext::cStateSuccess)
    {
        m_callerContext = 0;
        return kRun;
    }
    m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(&CloseParticipationJob::WaitP2pStable);
    m_currentStep.m_name = "CloseParticipationJob::WaitP2pStable";
    m_startTime.SetNow();
    return kStep;
}

unsigned int CloseParticipationJob::WaitP2pStable()
{
    const unsigned int failure = SessionFailureResult(g_piaSession->GetStatus());
    if (failure != 0)
    {
        m_callerContext->SignalFailure(static_cast<Result>(failure));
        m_callerContext = 0;
        return kRun;
    }

    const unsigned int elapsed = ElapsedMilliseconds(m_startTime);
    const unsigned int schedulerBits = elapsed & 0xff0000;
    if (elapsed > 1000 && IsP2pStable(m_sessionId))
    {
        if (g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex] == m_sessionId)
            g_piaSession->SetCurrentSessionOpenStatus(
                static_cast<Session::SessionOpenStatus>(2));
        else
            g_piaSession->SetBufferSessionOpenStatus(
                static_cast<Session::SessionOpenStatus>(2));
        m_callerContext->SignalSuccess(0);
        m_callerContext = 0;
        return kRun;
    }
    const unsigned int finalElapsed = ElapsedMilliseconds(m_startTime);
    if (finalElapsed > 15000)
    {
        m_callerContext->SignalFailure(static_cast<Result>(0x2c18));
        m_callerContext = 0;
        return (finalElapsed & 0xff0000) | kRun;
    }
    return schedulerBits | kWait;
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
