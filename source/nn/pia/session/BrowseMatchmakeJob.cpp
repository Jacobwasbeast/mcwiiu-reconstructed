#include "nn/pia/session/BrowseMatchmakeJob.hpp"

#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/SessionSearchCriteria.hpp"

namespace nn { namespace pia { namespace session {

BrowseMatchmakeJob::BrowseMatchmakeJob()
    : common::StepSequenceJob(), m_callContext(0)
{
}

BrowseMatchmakeJob::~BrowseMatchmakeJob()
{
}

Result BrowseMatchmakeJob::Startup(common::CallContext* callerContext,
                                   IMatchmakeSession* matchmakeSession,
                                   const SessionSearchCriteria* criteria)
{
    if (callerContext == 0 || criteria == 0)
        return static_cast<Result>(0x10c07);
    if (matchmakeSession == 0)
        return static_cast<Result>(0x10c08);
    if (m_callContext != 0)
        return static_cast<Result>(0x10c07);

    const Result result = StartupImpl(matchmakeSession, criteria);
    if (result != 0)
        return result;

    m_callContext = callerContext;
    callerContext->InitiateCall();
    Reset(true);
    return 0;
}

Result BrowseMatchmakeJob::Startup(common::CallContext* callerContext,
                                   IMatchmakeSession* matchmakeSession,
                                   unsigned int sessionId)
{
    if (callerContext == 0)
        return static_cast<Result>(0x10c07);
    if (matchmakeSession == 0)
        return static_cast<Result>(0x10c07);
    if (m_callContext != 0)
        return static_cast<Result>(0x10c07);

    const Result result = StartupImpl(matchmakeSession, sessionId);
    if (result != 0)
        return result;

    m_callContext = callerContext;
    callerContext->InitiateCall();
    Reset(true);
    return 0;
}

unsigned int BrowseMatchmakeJob::CompleteProcess()
{
    Result result = 0;
    m_callContext->SignalSuccess(result);
    m_callContext = 0;
    return common::Job::cExecuteResultFinished;
}

void BrowseMatchmakeJob::Cleanup()
{
    if (m_callContext != 0)
    {
        if (m_callContext->m_state == common::CallContext::cStateInProgress)
            m_callContext->SignalCancel();
        m_callContext = 0;
    }
}

void BrowseMatchmakeJob::Trace(unsigned long long) const
{
}

}}}
