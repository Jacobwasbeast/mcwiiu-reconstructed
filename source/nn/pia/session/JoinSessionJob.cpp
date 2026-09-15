#include "nn/pia/session/JoinSessionJob.hpp"

#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/session/JoinSessionSetting.hpp"

namespace nn { namespace pia { namespace session {

JoinSessionJob::JoinSessionJob()
    : common::StepSequenceJob(),
      m_callerContext(0),
      m_callContext(),
      m_stationConnectionInfo(),
      m_joinState(0),
      m_sessionId(0),
      m_joinOption(false),
      m_result(0),
      m_startTime()
{
}

JoinSessionJob::~JoinSessionJob()
{
}

void JoinSessionJob::Trace(unsigned long long value) const
{
}

Result JoinSessionJob::Startup(common::CallContext* callerContext,
                               const JoinSessionSetting* setting)
{
    if (callerContext == 0 || setting == 0)
        return 0x10c07;

    const Result result = StartupImpl(setting);
    if (result != 0)
        return result;

    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_sessionInvalidated = false;
    m_connectionInfoRetry = false;
    m_sessionId = setting->GetSessionId();
    m_result = 0;
    m_retryCount = 0;
    Reset(true);
    m_startTime = common::g_piaJobScheduler->m_dispatchTime;

    common::SessionBeginMonitoringContent& monitoring = common::g_sessionBeginMonitoringContent;
    monitoring.m_retryReasonBCount = 0xff;
    monitoring.m_retryReasonACount = 0xff;
    monitoring.m_sessionInfoRetryCount = 0xff;
    monitoring.m_ownerChangeRetryCount = 0xff;
    monitoring.m_connectionInfoRetryCount = 0xff;
    monitoring.m_joinRetryCount = 0xff;
    return 0;
}

void JoinSessionJob::Cleanup()
{
    CleanupImpl();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
    }
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
    m_ownerChanged = false;
    m_sessionId = 0;
    m_result = 0;
    m_connectionInfoRetry = false;
    m_sessionInfoRetry = false;
    m_joinOption = false;
    m_retryReasonA = false;
    m_joinState = 0;
    m_sessionInvalidated = false;
    m_retryReasonB = false;
    m_stationConnectionInfo.SetStationConnectionInfo(transport::StationConnectionInfo());
}

void JoinSessionJob::CleanupInternalContexts()
{
    m_callContext.SignalCancel();
    m_callContext.Reset();
}

bool JoinSessionJob::CheckContextCallCancelled()
{
    return m_callerContext != 0 && m_callerContext->m_cancelRequested;
}

unsigned int JoinSessionJob::GetJoinSessionStateForDebug() const
{
    return m_joinState;
}

void JoinSessionJob::NoticeOwnerChangeEvent(unsigned int, unsigned long long)
{
}

void JoinSessionJob::NoticeUnregisterEvent(unsigned int)
{
}

void JoinSessionJob::CleanupImpl()
{
}

}}}
