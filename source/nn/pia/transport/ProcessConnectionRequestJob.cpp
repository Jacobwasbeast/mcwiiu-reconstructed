#include "nn/pia/transport/ProcessConnectionRequestJob.hpp"

#include "nn/pia/common/Scheduler.hpp"

namespace nn { namespace pia { namespace transport {

ProcessConnectionRequestJob::ProcessConnectionRequestJob()
    : common::StepSequenceJob(), m_deadlineHigh(0), m_deadlineLow(0), m_timeoutMsec(10000),
      m_connectResult(0), m_responseResult(0), m_station(NULL),
      m_callContext(new common::CallContext()), m_relay(false)
{
}

ProcessConnectionRequestJob::~ProcessConnectionRequestJob()
{
    Cleanup();
    delete m_callContext;
    m_callContext = NULL;
}

bool ProcessConnectionRequestJob::Startup(Station* station, int timeoutMsec, bool relay)
{
    if (station == NULL)
        return false;
    m_station = station;
    m_timeoutMsec = timeoutMsec;
    m_relay = relay;
    m_connectResult = 0;
    m_responseResult = 0;
    if (m_callContext != NULL)
    {
        m_callContext->Reset();
        m_callContext->SignalSuccess(0);
    }
    return true;
}

bool ProcessConnectionRequestJob::StartupRelayConnection(Station* station, int timeoutMsec,
                                                           bool relay)
{
    return Startup(station, timeoutMsec, relay);
}

ProcessConnectionRequestJob::ExecuteResult ProcessConnectionRequestJob::ConnectToRequesterStation()
{
    if (m_station == NULL)
        return MakeExecuteResult(cExecuteKindFinished);
    return MakeExecuteResult(cExecuteKindNextStep);
}

ProcessConnectionRequestJob::ExecuteResult ProcessConnectionRequestJob::WaitInverseConnection()
{
    return MakeExecuteResult(cExecuteKindNextStep);
}

bool ProcessConnectionRequestJob::TimeOutCheck()
{
    const unsigned long long deadline =
        (static_cast<unsigned long long>(m_deadlineHigh) << 32) | m_deadlineLow;
    if (deadline <= common::g_piaJobScheduler->m_dispatchTime.m_tick)
    {
        m_callContext->Cancel();
        return true;
    }
    return false;
}

ProcessConnectionRequestJob::ExecuteResult ProcessConnectionRequestJob::SendConnectionResponse()
{
    return MakeExecuteResult(cExecuteKindNextStep);
}

ProcessConnectionRequestJob::ExecuteResult ProcessConnectionRequestJob::WaitResponseAck()
{
    Cleanup();
    return MakeExecuteResult(cExecuteKindFinished);
}

void ProcessConnectionRequestJob::Cleanup()
{
    m_station = NULL;
    m_connectResult = 0;
    m_responseResult = 0;
}

void ProcessConnectionRequestJob::Trace(unsigned long long) const
{
}

}}}
