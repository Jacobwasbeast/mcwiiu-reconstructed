#include "nn/pia/transport/ProcessConnectionRequestJob.hpp"

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

unsigned int ProcessConnectionRequestJob::Startup(Station* station, int timeoutMsec, bool relay)
{
    if (station == NULL)
        return common::Job::cExecuteResultFinished;
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
    return common::Job::cExecuteResultNextStep;
}

unsigned int ProcessConnectionRequestJob::StartupRelayConnection(Station* station, int timeoutMsec,
                                                                   bool relay)
{
    return Startup(station, timeoutMsec, relay);
}

unsigned int ProcessConnectionRequestJob::ConnectToRequesterStation()
{
    if (m_station == NULL)
        return common::Job::cExecuteResultFinished;
    return common::Job::cExecuteResultNextStep;
}

unsigned int ProcessConnectionRequestJob::WaitInverseConnection()
{
    return common::Job::cExecuteResultNextStep;
}

unsigned int ProcessConnectionRequestJob::TimeOutCheck()
{
    return common::Job::cExecuteResultNextStep;
}

unsigned int ProcessConnectionRequestJob::SendConnectionResponse()
{
    return common::Job::cExecuteResultNextStep;
}

unsigned int ProcessConnectionRequestJob::WaitResponseAck()
{
    Cleanup();
    return common::Job::cExecuteResultFinished;
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
