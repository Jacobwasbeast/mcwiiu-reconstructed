#include "nn/pia/transport/DisconnectStationJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/transport/ProcessConnectionRequestJob.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationProtocol.hpp"

namespace nn { namespace pia { namespace transport {

DisconnectStationJob::DisconnectStationJob()
    : common::StepSequenceJob(), m_deadline(), m_timeoutMsec(4000), m_station(0),
      m_retryDeadline(), m_retryIntervalMsec(500), m_waiting(false)
{
}

DisconnectStationJob::~DisconnectStationJob()
{
}

Result DisconnectStationJob::Startup(Station* station)
{
    return StartupImpl(station);
}

unsigned int DisconnectStationJob::SendDisconnectionRequest()
{
    if (static_cast<int>(m_station->m_field10) <= StationManager::cStationNumMax - 1)
    {
        m_station->m_field28->SendDisconnectionRequest(
            static_cast<StationIndex>(m_station->m_field10), false);
    }
    else
    {
        if (!m_station->m_stationAddress.IsValid())
        {
            m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
                &DisconnectStationJob::DisconnectionSucceeded);
            m_currentStep.m_name = "DisconnectStationJob::DisconnectionSucceeded";
            return cExecuteResultContinue;
        }
        m_station->m_field28->SendDisconnectionRequest(m_station->m_stationAddress);
    }
    m_waiting = true;
    m_retryDeadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick +
        common::TimeSpan::GetTicksPerMSec().m_tick * m_retryIntervalMsec;
    m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
        &DisconnectStationJob::WaitForDisconnection);
    m_currentStep.m_name = "DisconnectStationJob::WaitForDisconnection";
    return cExecuteResultNextStep;
}

unsigned int DisconnectStationJob::WaitForDisconnection()
{
    if (!m_waiting)
    {
        m_station->m_field20 = 6;
        m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
            &DisconnectStationJob::DisconnectionSucceeded);
        m_currentStep.m_name = "DisconnectStationJob::DisconnectionSucceeded";
        return cExecuteResultContinue;
    }
    if (m_deadline <= common::g_piaJobScheduler->m_dispatchTime ||
        (static_cast<int>(m_station->m_field10) > StationManager::cStationNumMax - 1 &&
         !m_station->m_stationAddress.IsValid()))
    {
        m_waiting = false;
        m_station->m_field20 = 6;
        m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
            &DisconnectStationJob::DisconnectionSucceeded);
        m_currentStep.m_name = "DisconnectStationJob::DisconnectionSucceeded";
        return cExecuteResultContinue;
    }
    if (common::g_piaJobScheduler->m_dispatchTime < m_retryDeadline)
        return cExecuteResultNextStep;
    if (static_cast<int>(m_station->m_field10) <= StationManager::cStationNumMax - 1)
    {
        m_station->m_field28->SendDisconnectionRequest(
            static_cast<StationIndex>(m_station->m_field10), false);
    }
    else
    {
        m_station->m_field28->SendDisconnectionRequest(m_station->m_stationAddress);
    }
    m_retryDeadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick +
        common::TimeSpan::GetTicksPerMSec().m_tick * m_retryIntervalMsec;
    return cExecuteResultNextStep;
}

unsigned int DisconnectStationJob::DisconnectionSucceeded()
{
    m_station->m_disconnectStationJob->ProcessDisconnection(m_station);
    g_stationConnectionInfoTable->EraseFromTable(m_station);
    IdentificationInfoTable::GetInstance()->EraseFromTable(m_station);
    ConnectStationJob* connectJob = m_station->m_connectStationJob;
    if (connectJob)
    {
        connectJob->Cleanup();
        connectJob->Reset(false);
    }
    ProcessConnectionRequestJob* processJob = m_station->m_processConnectionRequestJob;
    if (processJob)
    {
        processJob->Cleanup();
        processJob->Reset(false);
    }
    m_station->Cleanup();
    g_piaStationManager->DestroyStation(m_station);
    return cExecuteResultFinished;
}

unsigned int DisconnectStationJob::CutRouteOfRelayConnection()
{
    if (static_cast<int>(m_station->m_field10) <= StationManager::cStationNumMax - 1)
    {
        m_station->m_field28->SendDisconnectionRequest(
            static_cast<StationIndex>(m_station->m_field10), true);
        m_station->m_field28->SendDisconnectionRequest(
            static_cast<StationIndex>(m_station->m_field10), true);
    }
    m_station->m_field20 = 6;
    m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
        &DisconnectStationJob::DisconnectionSucceeded);
    m_currentStep.m_name = "DisconnectStationJob::DisconnectionSucceeded";
    return cExecuteResultContinue;
}

void DisconnectStationJob::Cleanup()
{
    m_waiting = false;
    m_deadline.m_tick = 0;
    m_station = 0;
}

void DisconnectStationJob::Trace(unsigned long long) const
{
}

void DisconnectStationJob::ProcessDisconnection(Station*)
{
}

Result DisconnectStationJob::StartupImpl(Station* station)
{
    if (!station)
        return 0x10c07;
    if (station->m_field20 != 5)
        return 0x10c08;
    m_station = station;
    m_waiting = false;
    common::Time now;
    now.SetNow();
    m_deadline.m_tick = now.m_tick + m_timeoutMsec * common::TimeSpan::GetTicksPerMSec().m_tick;
    Reset(true);
    m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
        &DisconnectStationJob::SendDisconnectionRequest);
    m_currentStep.m_name = "DisconnectStationJob::SendDisconnectionRequest";
    if (g_piaTransport->m_relayRouteManager && g_piaStationManager->m_localStation)
    {
        StationIndex relay;
        Result result = g_piaTransport->m_relayRouteManager->GetRelayRoute(
            static_cast<StationIndex>(g_piaStationManager->m_localStation->m_field10),
            static_cast<StationIndex>(m_station->m_field10), &relay);
        if (result == 0 && static_cast<unsigned int>(relay) != m_station->m_field10)
        {
            m_currentStep.m_function = static_cast<unsigned int (common::StepSequenceJob::*)()>(
                &DisconnectStationJob::CutRouteOfRelayConnection);
            m_currentStep.m_name = "DisconnectStationJob::CutRouteOfRelayConnection";
        }
    }
    return 0;
}

}}}
