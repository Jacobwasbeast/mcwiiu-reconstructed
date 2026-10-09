#include "nn/pia/inet/NexMatchBrowseMatchmakeJob.hpp"

#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/pia/inet/NexSessionSearchCriteriaOwner.hpp"
#include "nn/pia/inet/NexSessionSearchCriteriaParticipant.hpp"
#include "nn/pia/inet/NexSessionSearchCriteriaSessionId.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/SessionSearchCriteria.hpp"

namespace nn { namespace pia { namespace inet {

namespace
{
enum
{
    cResultRequestFailed = 0xac5a,
    cResultInvalidArgument = 0x10c07
};

enum
{
    cBrowseSearchCriteriaNum = 1
};

inline bool signalSessionStatusFailure(common::CallContext*& callerContext)
{
    switch (session::g_piaSession->GetStatus())
    {
    case session::Session::cStatusDisconnectedNetwork:
        callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        callerContext = 0;
        return true;
    case session::Session::cStatusUnknown6:
        callerContext->SignalFailure(session::Session::cResultStatusUnknown6);
        callerContext = 0;
        return true;
    case session::Session::cStatusUnknown7:
        callerContext->SignalFailure(session::Session::cResultStatusUnknown7);
        callerContext = 0;
        return true;
    case session::Session::cStatusUnknown8:
        callerContext->SignalFailure(session::Session::cResultStatusUnknown8);
        callerContext = 0;
        return true;
    default:
        return false;
    }
}
}

NexMatchBrowseMatchmakeJob::NexMatchBrowseMatchmakeJob()
    : session::BrowseMatchmakeJob(), m_callContext(), m_matchmakeSession(0), m_startupResult(0)
{
}

NexMatchBrowseMatchmakeJob::~NexMatchBrowseMatchmakeJob()
{
}

void NexMatchBrowseMatchmakeJob::Cleanup()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
    if (m_matchmakeSession != 0)
    {
        m_matchmakeSession->UnregisterSearchCriteria();
        m_matchmakeSession = 0;
    }
    session::BrowseMatchmakeJob::Cleanup();
}

NexMatchBrowseMatchmakeJob::ExecuteResult NexMatchBrowseMatchmakeJob::BrowseMatchmake()
{
    if (signalSessionStatusFailure(m_callerContext))
        return MakeExecuteResult(cExecuteKindFinished);
    const Result result = m_matchmakeSession->BrowseMatchmakeSession(&m_callContext);
    if (result != 0)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(result);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SetStep(&NexMatchBrowseMatchmakeJob::WaitBrowseMatchmake,
            "NexMatchBrowseMatchmakeJob::WaitBrowseMatchmake");
    return MakeExecuteResult(cExecuteKindContinue);
}

NexMatchBrowseMatchmakeJob::ExecuteResult NexMatchBrowseMatchmakeJob::WaitBrowseMatchmake()
{
    if (signalSessionStatusFailure(m_callerContext))
        return MakeExecuteResult(cExecuteKindFinished);
    if (m_matchmakeSession->WaitBrowseMatchmakeSession())
    {
        m_matchmakeSession->UnregisterSearchCriteria();
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            SetStep(&session::BrowseMatchmakeJob::CompleteProcess,
                    "session::BrowseMatchmakeJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchBrowseMatchmakeJob::ExecuteResult NexMatchBrowseMatchmakeJob::WaitFindSessionByOwner()
{
    if (signalSessionStatusFailure(m_callerContext))
        return MakeExecuteResult(cExecuteKindFinished);
    if (m_startupResult != 0)
    {
        m_callerContext->SignalFailure(m_startupResult);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_matchmakeSession->WaitFindSessionByOwner())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            SetStep(&session::BrowseMatchmakeJob::CompleteProcess,
                    "session::BrowseMatchmakeJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchBrowseMatchmakeJob::ExecuteResult NexMatchBrowseMatchmakeJob::WaitFindSessionByParticipant()
{
    if (signalSessionStatusFailure(m_callerContext))
        return MakeExecuteResult(cExecuteKindFinished);
    if (m_startupResult != 0)
    {
        m_callerContext->SignalFailure(m_startupResult);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_matchmakeSession->WaitFindSessionByParticipant())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            SetStep(&session::BrowseMatchmakeJob::CompleteProcess,
                    "session::BrowseMatchmakeJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchBrowseMatchmakeJob::ExecuteResult NexMatchBrowseMatchmakeJob::WaitFindSessionBySessionId()
{
    if (signalSessionStatusFailure(m_callerContext))
        return MakeExecuteResult(cExecuteKindFinished);
    if (m_startupResult != 0)
    {
        m_callerContext->SignalFailure(m_startupResult);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_matchmakeSession->WaitFindSessionBySessionId())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            SetStep(&session::BrowseMatchmakeJob::CompleteProcess,
                    "session::BrowseMatchmakeJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchBrowseMatchmakeJob::ExecuteResult NexMatchBrowseMatchmakeJob::WaitRequestSessionInfo()
{
    if (signalSessionStatusFailure(m_callerContext))
        return MakeExecuteResult(cExecuteKindFinished);
    if (m_startupResult != 0)
    {
        m_callerContext->SignalFailure(m_startupResult);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_matchmakeSession->WaitRequestSessionInfo())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            SetStep(&session::BrowseMatchmakeJob::CompleteProcess,
                    "session::BrowseMatchmakeJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void NexMatchBrowseMatchmakeJob::Trace(unsigned long long) const
{
}

Result NexMatchBrowseMatchmakeJob::StartupImpl(session::IMatchmakeSession* matchmakeSession,
                                               const session::SessionSearchCriteria* criteria)
{
    m_matchmakeSession = static_cast<NexMatchmakeSession*>(matchmakeSession);
    if (criteria->GetKeyType() == session::SessionSearchCriteria::cKeyTypeOwner)
    {
        m_startupResult = m_matchmakeSession->FindSessionByOwner(
            &m_callContext, *static_cast<const NexSessionSearchCriteriaOwner*>(criteria));
        if (m_startupResult != 0 && m_startupResult != cResultRequestFailed)
            return m_startupResult;
        SetStep(&NexMatchBrowseMatchmakeJob::WaitFindSessionByOwner,
                "NexMatchBrowseMatchmakeJob::WaitFindSessionByOwner");
    }
    else if (criteria->GetKeyType() == session::SessionSearchCriteria::cKeyTypeParticipant)
    {
        m_startupResult = m_matchmakeSession->FindSessionByParticipant(
            &m_callContext, *static_cast<const NexSessionSearchCriteriaParticipant*>(criteria));
        if (m_startupResult != 0 && m_startupResult != cResultRequestFailed)
            return m_startupResult;
        SetStep(&NexMatchBrowseMatchmakeJob::WaitFindSessionByParticipant,
                "NexMatchBrowseMatchmakeJob::WaitFindSessionByParticipant");
    }
    else if (criteria->GetKeyType() == session::SessionSearchCriteria::cKeyTypeSessionId)
    {
        m_startupResult = m_matchmakeSession->FindSessionBySessionId(
            &m_callContext, *static_cast<const NexSessionSearchCriteriaSessionId*>(criteria));
        if (m_startupResult != 0 && m_startupResult != cResultRequestFailed)
            return m_startupResult;
        SetStep(&NexMatchBrowseMatchmakeJob::WaitFindSessionBySessionId,
                "NexMatchBrowseMatchmakeJob::WaitFindSessionBySessionId");
    }
    else
    {
        if (m_matchmakeSession->RegisterSearchCriteria(criteria, cBrowseSearchCriteriaNum) != 0)
            return cResultInvalidArgument;
        SetStep(&NexMatchBrowseMatchmakeJob::BrowseMatchmake, "NexMatchBrowseMatchmakeJob::BrowseMatchmake");
    }
    return 0;
}

Result NexMatchBrowseMatchmakeJob::StartupImpl(session::IMatchmakeSession* matchmakeSession,
                                               unsigned int sessionId)
{
    m_matchmakeSession = static_cast<NexMatchmakeSession*>(matchmakeSession);
    m_startupResult = m_matchmakeSession->RequestSessionInfo(&m_callContext, sessionId);
    if (m_startupResult != 0 && m_startupResult != cResultRequestFailed)
        return m_startupResult;
    SetStep(&NexMatchBrowseMatchmakeJob::WaitRequestSessionInfo,
            "NexMatchBrowseMatchmakeJob::WaitRequestSessionInfo");
    return 0;
}

}}}
