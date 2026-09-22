#include "nn/pia/transport/DisconnectStationJob.hpp"

#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn { namespace pia { namespace transport {

namespace
{
typedef unsigned int (DisconnectStationJob::*DisconnectStep)();

void SelectStep(common::StepSequenceJob::Step& slot, DisconnectStep step)
{
    union
    {
        DisconnectStep derived;
        unsigned int (common::StepSequenceJob::*base)();
    } value;
    value.derived = step;
    slot.m_function = value.base;
}

unsigned int Now()
{
    common::Time time;
    time.SetNow();
    return static_cast<unsigned int>(time.m_tick);
}
}

DisconnectStationJob::DisconnectStationJob()
    : common::StepSequenceJob(), m_station(0), m_deadline(0), m_retryDeadline(0),
      m_timeout(4000), m_retryInterval(500), m_waiting(0), m_padding()
{
}

DisconnectStationJob::~DisconnectStationJob()
{
}

void DisconnectStationJob::Startup(Station* station)
{
    StartupImpl(station);
}

unsigned int DisconnectStationJob::SendDisconnectionRequest()
{
    if (m_station == 0)
        return DisconnectionSucceeded();

    m_waiting = 1;
    m_retryDeadline = Now() + m_retryInterval;
    m_deadline = Now() + m_timeout;
    SelectStep(m_currentStep, &DisconnectStationJob::WaitForDisconnection);
    return cExecuteResultWait;
}

unsigned int DisconnectStationJob::WaitForDisconnection()
{
    if (m_station == 0 || m_waiting == 0 || Now() >= m_deadline)
    {
        if (m_station != 0)
            m_station->m_field20 = 6;
        m_waiting = 0;
        SelectStep(m_currentStep, &DisconnectStationJob::DisconnectionSucceeded);
        return cExecuteResultNextStep;
    }

    if (Now() >= m_retryDeadline)
        m_retryDeadline = Now() + m_retryInterval;
    return cExecuteResultWait;
}

unsigned int DisconnectStationJob::DisconnectionSucceeded()
{
    Station* station = m_station;
    if (station != 0)
    {
        station->Cleanup();
        if (g_piaStationManager != 0)
            g_piaStationManager->DestroyStation(station);
    }
    Cleanup();
    return cExecuteResultFinished;
}

unsigned int DisconnectStationJob::CutRouteOfRelayConnection()
{
    if (m_station != 0)
        m_station->m_field20 = 6;
    m_waiting = 0;
    SelectStep(m_currentStep, &DisconnectStationJob::DisconnectionSucceeded);
    return cExecuteResultNextStep;
}

void DisconnectStationJob::Cleanup()
{
    m_waiting = 0;
    m_retryDeadline = 0;
    m_deadline = 0;
    m_station = 0;
}

void DisconnectStationJob::Trace(unsigned long long) const
{
}

unsigned int DisconnectStationJob::StartupImpl(Station* station)
{
    m_station = station;
    m_waiting = 0;
    m_deadline = 0;
    m_retryDeadline = 0;
    Reset(true);
    SelectStep(m_currentStep, &DisconnectStationJob::SendDisconnectionRequest);
    return cExecuteResultNextStep;
}

void DisconnectStationJob::ProcessDisconnection(Station*)
{
}

}}}
