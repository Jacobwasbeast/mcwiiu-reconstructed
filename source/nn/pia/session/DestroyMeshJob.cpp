#include "nn/pia/session/DestroyMeshJob.hpp"

#include <coreinit/memory.h>

#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace nn { namespace pia { namespace session {

DestroyMeshJob::DestroyMeshJob()
    : common::StepSequenceJob(), m_callerContext(0), m_deadline(),
      m_timeoutMsec(cResponseTimeoutMsec), m_waitingResponses(false), m_started(false),
      m_systemDestroy(false)
{
    OSBlockSet(m_stationResponses, 0, sizeof(m_stationResponses));
}

DestroyMeshJob::~DestroyMeshJob()
{
}

bool DestroyMeshJob::Startup(common::CallContext* callContext, bool systemDestroy)
{
    if (m_started)
        return false;

    if (callContext != 0)
    {
        m_callerContext = callContext;
        callContext->InitiateCall();
    }

    const int timeoutMsec = m_timeoutMsec;
    m_systemDestroy = systemDestroy;
    m_deadline = common::Time::Create(common::g_piaJobScheduler->m_dispatchTime.m_tick +
                                      common::TimeSpan::GetTicksPerMSec().m_tick * timeoutMsec);
    m_started = true;

    unsigned int reason = 2;
    if (systemDestroy)
        reason = g_piaMesh->m_disconnectReason;
    g_piaMesh->ProcessingAtMeshEnd(static_cast<Mesh::DisconnectReason>(reason));

    Reset(true);
    setStep(&DestroyMeshJob::SendDestroyMesh, "DestroyMeshJob::SendDestroyMesh");
    return true;
}

bool DestroyMeshJob::AssociateSystemWith(common::CallContext* callContext)
{
    if (!m_started || !m_systemDestroy)
        return false;

    if (callContext != 0)
    {
        m_callerContext = callContext;
        callContext->InitiateCall();
    }
    m_systemDestroy = false;
    return true;
}

unsigned int DestroyMeshJob::SendDestroyMesh()
{
    const int localStationIndex = g_piaMesh->GetLocalStationIndex();
    if (localStationIndex >= cStationNumMax)
    {
        setStep(&DestroyMeshJob::CleanupMesh, "DestroyMeshJob::CleanupMesh");
        return cExecuteResultContinue;
    }

    for (int i = 0; i < cStationNumMax; ++i)
    {
        if (i == localStationIndex || !g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(i)))
        {
            m_stationResponses[i] = 0;
        }
        else
        {
            g_piaMesh->GetMeshProtocol()->SendDestroyMesh(static_cast<StationIndex>(i));
            m_stationResponses[i] = 1;
        }
    }

    m_waitingResponses = true;
    setStep(&DestroyMeshJob::WaitDestroyResponse, "DestroyMeshJob::WaitDestroyResponse");
    return cExecuteResultNextStep;
}

unsigned int DestroyMeshJob::WaitDestroyResponse()
{
    bool pending = false;
    for (int i = 0; i < cStationNumMax; ++i)
    {
        if (m_stationResponses[i] != 0)
        {
            pending = true;
            break;
        }
    }

    if (pending && common::g_piaJobScheduler->m_dispatchTime < m_deadline)
        return cExecuteResultNextStep;

    setStep(&DestroyMeshJob::CleanupMesh, "DestroyMeshJob::CleanupMesh");
    m_waitingResponses = false;
    return cExecuteResultContinue;
}

void DestroyMeshJob::CleanupStatus()
{
    m_callerContext = 0;
    OSBlockSet(m_stationResponses, 0, sizeof(m_stationResponses));
    m_waitingResponses = false;
    m_started = false;
    m_systemDestroy = false;
}

unsigned int DestroyMeshJob::CleanupMesh()
{
    transport::Station** station = transport::g_piaStationManager->m_stations.data;
    transport::Station** const end = station + transport::g_piaStationManager->m_stations.size;
    for (; station != end; ++station)
        (*station)->m_disconnectStationJob->ProcessDisconnection(*station);

    g_piaMesh->CleanupStationsJobs();
    if (!m_systemDestroy)
        g_piaMesh->m_disconnectReason = 2;
    g_piaMesh->CleanupStatus();

    if (m_callerContext != 0)
    {
        m_callerContext->SignalSuccess(Result(0));
        m_callerContext = 0;
    }
    CleanupStatus();
    return cExecuteResultFinished;
}

void DestroyMeshJob::ReceiveDestroyResponse(StationIndex stationIndex)
{
    if (static_cast<int>(stationIndex) >= cStationNumMax)
        return;
    m_stationResponses[stationIndex] = 0;
}

void DestroyMeshJob::Cleanup()
{
    common::CallContext* callerContext = m_callerContext;
    m_deadline = common::Time();
    if (callerContext != 0)
    {
        if (callerContext->m_state == common::CallContext::cStateInProgress)
            callerContext->SignalSuccess(Result(0));
        m_callerContext = 0;
    }
    CleanupStatus();
}

void DestroyMeshJob::Trace(unsigned long long value) const
{
}

}}}
