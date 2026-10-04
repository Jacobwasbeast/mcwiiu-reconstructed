#include "nn/pia/session/LeaveWithHostMigrationJob.hpp"

#include <coreinit/memory.h>

#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
enum
{
    cDefaultTimeoutMsec = 5000,
    cDisconnectReason2 = 2,
    cStationState5 = 5,
    cTraceMask1 = 0x10000000
};

const char* const cStepNameCheckHostMigrationProcess =
    "LeaveWithHostMigrationJob::CheckHostMigrationProcess";
const char* const cStepNameSendStartMigrationMessage =
    "LeaveWithHostMigrationJob::SendStartMigrationMessage";
const char* const cStepNameSendStartMultiMigrationMessage =
    "LeaveWithHostMigrationJob::SendStartMultiMigrationMessage";
const char* const cStepNameWaitMigrationResponse =
    "LeaveWithHostMigrationJob::WaitMigrationResponse";
const char* const cStepNameWaitHostMigrationProcess =
    "LeaveWithHostMigrationJob::WaitHostMigrationProcess";
const char* const cStepNameStartDisconnectStations =
    "LeaveWithHostMigrationJob::StartDisconnectStations";
const char* const cStepNameWaitDisconnectStations =
    "LeaveWithHostMigrationJob::WaitDisconnectStations";
const char* const cStepNameCleanupMesh = "LeaveWithHostMigrationJob::CleanupMesh";
}

LeaveWithHostMigrationJob::LeaveWithHostMigrationJob()
    : common::StepSequenceJob(), m_callerContext(0), m_deadline(), m_timeoutMsec(cDefaultTimeoutMsec),
      m_started(false), m_newHost(static_cast<StationIndex>(ProcessHostMigrationJob::cStationIndexInvalid))
{
    OSBlockSet(m_stationResponses, 0, sizeof(m_stationResponses));
    m_migrationStarted = false;
}

LeaveWithHostMigrationJob::~LeaveWithHostMigrationJob()
{
}

bool LeaveWithHostMigrationJob::Startup(common::CallContext* context)
{
    if (m_started)
        return false;
    const common::TimeSpan& ticksPerMSec = common::TimeSpan::GetTicksPerMSec();
    m_deadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick +
        ticksPerMSec.m_tick * m_timeoutMsec;
    if (context != 0)
    {
        m_callerContext = context;
        context->InitiateCall();
    }
    m_started = true;
    g_piaMesh->ProcessingAtMeshEnd(static_cast<Mesh::DisconnectReason>(cDisconnectReason2));
    Reset(true);
    setStep(&LeaveWithHostMigrationJob::CheckHostMigrationProcess, cStepNameCheckHostMigrationProcess);
    return true;
}

unsigned int LeaveWithHostMigrationJob::SendStartMigrationMessage()
{
    m_newHost = static_cast<StationIndex>(DecideNextHost());
    const StationIndex localIndex = g_piaMesh->GetLocalStationIndex();
    if (m_newHost > ProcessHostMigrationJob::cStationIndexMax || m_newHost == localIndex)
    {
        setStep(&LeaveWithHostMigrationJob::CleanupMesh, cStepNameCleanupMesh);
        return cExecuteResultContinue;
    }
    for (int index = 0; index < cStationNumMax; ++index)
    {
        if (index == localIndex || !g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(index)))
        {
            m_stationResponses[index] = false;
        }
        else
        {
            g_piaMesh->GetMeshProtocol()->SendMigrationStart(static_cast<StationIndex>(index), m_newHost);
            m_stationResponses[index] = true;
        }
    }
    m_migrationStarted = true;
    setStep(&LeaveWithHostMigrationJob::WaitMigrationResponse, cStepNameWaitMigrationResponse);
    return cExecuteResultNextStep;
}

unsigned int LeaveWithHostMigrationJob::WaitMigrationResponse()
{
    for (int index = 0; index < cStationNumMax; ++index)
    {
        if (!g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(index)))
        {
            m_stationResponses[index] = false;
        }
        else if (m_stationResponses[index])
        {
            if (common::g_piaJobScheduler->m_dispatchTime < m_deadline)
                return cExecuteResultNextStep;
            break;
        }
    }
    m_migrationStarted = false;
    setStep(&LeaveWithHostMigrationJob::CleanupMesh, cStepNameCleanupMesh);
    return cExecuteResultContinue;
}

unsigned int LeaveWithHostMigrationJob::CleanupMesh()
{
    transport::StationManager* manager = transport::g_piaStationManager;
    for (transport::Station** it = manager->m_stations.data;
         it != manager->m_stations.data + manager->m_stations.size; ++it)
    {
        transport::Station* station = *it;
        station->m_disconnectStationJob->ProcessDisconnection(station);
    }
    g_piaMesh->CleanupStationsJobs();
    g_piaMesh->m_disconnectReason = cDisconnectReason2;
    g_piaMesh->CleanupStatus();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalSuccess(0);
        m_callerContext = 0;
    }
    CleanupStatus();
    return cExecuteResultFinished;
}

unsigned int LeaveWithHostMigrationJob::SendStartMultiMigrationMessage()
{
    if (!g_piaMesh->GetMeshProtocol()->SendMultiMigrationStart(m_stationResponses))
    {
        setStep(&LeaveWithHostMigrationJob::CleanupMesh, cStepNameCleanupMesh);
        return cExecuteResultContinue;
    }
    m_migrationStarted = true;
    setStep(&LeaveWithHostMigrationJob::WaitMigrationResponse, cStepNameWaitMigrationResponse);
    return cExecuteResultNextStep;
}

unsigned int LeaveWithHostMigrationJob::CheckHostMigrationProcess()
{
    ProcessHostMigrationJob* migrationJob = g_piaMesh->m_processHostMigrationJob;
    const bool isMultiMigration = g_piaMesh->m_hostMigrationMode == 2;
    if (!migrationJob->m_started)
    {
        if (!isMultiMigration)
        {
            setStep(&LeaveWithHostMigrationJob::SendStartMigrationMessage,
                    cStepNameSendStartMigrationMessage);
            return cExecuteResultContinue;
        }
        setStep(&LeaveWithHostMigrationJob::SendStartMultiMigrationMessage,
                cStepNameSendStartMultiMigrationMessage);
        return cExecuteResultContinue;
    }
    if (!isMultiMigration)
    {
        setStep(&LeaveWithHostMigrationJob::WaitHostMigrationProcess,
                cStepNameWaitHostMigrationProcess);
        return cExecuteResultNextStep;
    }
    migrationJob->Cleanup();
    migrationJob->m_cancelRequested = true;
    setStep(&LeaveWithHostMigrationJob::StartDisconnectStations, cStepNameStartDisconnectStations);
    return cExecuteResultContinue;
}

unsigned int LeaveWithHostMigrationJob::WaitHostMigrationProcess()
{
    if (!g_piaMesh->m_processHostMigrationJob->m_started)
    {
        setStep(&LeaveWithHostMigrationJob::SendStartMigrationMessage,
                cStepNameSendStartMigrationMessage);
        return cExecuteResultContinue;
    }
    if (m_deadline <= common::g_piaJobScheduler->m_dispatchTime)
    {
        setStep(&LeaveWithHostMigrationJob::CleanupMesh, cStepNameCleanupMesh);
        return cExecuteResultContinue;
    }
    return cExecuteResultNextStep;
}

unsigned int LeaveWithHostMigrationJob::StartDisconnectStations()
{
    transport::StationManager* manager = transport::g_piaStationManager;
    transport::Station* localStation = manager->m_localStation;
    if (localStation == 0)
    {
        setStep(&LeaveWithHostMigrationJob::CleanupMesh, cStepNameCleanupMesh);
        return cExecuteResultContinue;
    }
    for (transport::Station** it = manager->m_stations.data;
         it != manager->m_stations.data + manager->m_stations.size; ++it)
    {
        transport::Station* station = *it;
        if (station != localStation)
        {
            const StationIndex stationIndex = static_cast<StationIndex>(station->m_field10);
            if (g_piaMesh->IsValidStationIndex(stationIndex))
                g_piaMesh->UnfixDisconnectedId(stationIndex);
            transport::DisconnectStationJob* disconnectJob = station->m_disconnectStationJob;
            if (disconnectJob == 0)
            {
                station->Trace(cTraceMask1);
            }
            else
            {
                const Result result = disconnectJob->Startup(station);
                if (result.m_code == 0)
                    disconnectJob->Ready(false);
                else
                    station->Trace(cTraceMask1);
            }
        }
    }
    setStep(&LeaveWithHostMigrationJob::WaitDisconnectStations, cStepNameWaitDisconnectStations);
    return cExecuteResultContinue;
}

unsigned int LeaveWithHostMigrationJob::WaitDisconnectStations()
{
    transport::StationManager* manager = transport::g_piaStationManager;
    for (transport::Station** it = manager->m_stations.data;
         it != manager->m_stations.data + manager->m_stations.size; ++it)
    {
        transport::Station* station = *it;
        if (station->m_field20 == cStationState5 && station != manager->m_localStation)
        {
            if (common::g_piaJobScheduler->m_dispatchTime < m_deadline)
                return cExecuteResultNextStep;
            break;
        }
    }
    setStep(&LeaveWithHostMigrationJob::CleanupMesh, cStepNameCleanupMesh);
    return cExecuteResultContinue;
}

void LeaveWithHostMigrationJob::ReceiveMigrationResponse(StationIndex station)
{
    if (static_cast<int>(station) > ProcessHostMigrationJob::cStationIndexMax)
        return;
    m_stationResponses[station] = false;
}

void LeaveWithHostMigrationJob::Cleanup()
{
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
    }
    CleanupStatus();
    m_deadline.m_tick = 0;
}

void LeaveWithHostMigrationJob::Trace(unsigned long long) const
{
}

void LeaveWithHostMigrationJob::CleanupStatus()
{
    m_callerContext = 0;
    m_newHost = static_cast<StationIndex>(ProcessHostMigrationJob::cStationIndexInvalid);
    m_started = false;
    OSBlockSet(m_stationResponses, 0, sizeof(m_stationResponses));
    m_migrationStarted = false;
}

}}}
