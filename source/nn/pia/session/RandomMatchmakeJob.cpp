#include "nn/pia/session/RandomMatchmakeJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
enum RandomMatchmakeResult
{
    cResultSessionDisconnected = 0xc406,
    cResultSessionStatus6 = 0xa471,
    cResultSessionStatus7 = 0xc420,
    cResultSessionStatus8 = 0xc421,
    cResultJoinFailed = 0x6439,
    cResultCancelled = 0x6c05,
    cResultCreateFailed = 0x10c08,
    cResultInvalidArgument = 0x10c07
};

enum SessionStatus
{
    cSessionStatusDisconnected = 5,
    cSessionStatus6 = 6,
    cSessionStatus7 = 7,
    cSessionStatus8 = 8
};

enum
{
    cJoinStateWaitJoinMesh = 4,
    cJoinStateComplete = 9,
    cJoinMeshStateUnknown3 = 3,
    cMonitoringByteUnset = 0xff
};

inline unsigned int FailureForStatus(unsigned int status)
{
    switch (status)
    {
    case cSessionStatusDisconnected:
        return cResultSessionDisconnected;
    case cSessionStatus6:
        return cResultSessionStatus6;
    case cSessionStatus7:
        return cResultSessionStatus7;
    case cSessionStatus8:
        return cResultSessionStatus8;
    default:
        return 0;
    }
}

inline bool FailForSessionStatus(RandomMatchmakeJob& job)
{
    const unsigned int failure = FailureForStatus(g_piaSession->GetStatus());
    if (failure == 0)
        return false;
    job.setStep(&RandomMatchmakeJob::CompleteFailure, "nn::pia::session::RandomMatchmakeJob::CompleteFailure");
    job.m_result = failure;
    return true;
}
}

RandomMatchmakeJob::RandomMatchmakeJob()
    : common::StepSequenceJob(),
      m_callerContext(0),
      m_joinState(0),
      m_unknown48(0),
      m_unknown4c(false),
      m_unknown4d(false),
      m_result(0),
      m_stationConnectionInfo(),
      m_callContext(),
      m_startTime()
{
}

RandomMatchmakeJob::~RandomMatchmakeJob()
{
}

Result RandomMatchmakeJob::Startup(common::CallContext* callerContext,
                                   const JoinRandomSessionSetting* setting)
{
    if (callerContext == 0 || setting == 0)
        return cResultInvalidArgument;
    if (m_callerContext != 0)
        return cResultInvalidArgument;

    const Result result = StartupImpl(setting);
    if (result != 0)
        return result;

    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_unknown5a = false;
    m_unknown58 = false;
    m_unknown57 = false;
    m_unknown54 = false;
    m_result = 0;
    Reset(true);

    common::SessionBeginMonitoringContent& monitoring = common::g_sessionBeginMonitoringContent;
    monitoring.m_connectionInfoRetryCount = cMonitoringByteUnset;
    monitoring.m_joinRetryCount = cMonitoringByteUnset;
    monitoring.m_retryReasonBCount = cMonitoringByteUnset;
    monitoring.m_ownerChangeRetryCount = cMonitoringByteUnset;
    monitoring.m_sessionInfoRetryCount = cMonitoringByteUnset;
    monitoring.m_retryReasonACount = cMonitoringByteUnset;
    m_startTime = common::g_piaJobScheduler->m_dispatchTime;
    return 0;
}

unsigned int RandomMatchmakeJob::MeshStartup()
{
    if (FailForSessionStatus(*this))
        return cExecuteResultContinue;

    if (m_unknown57)
    {
        const bool createMesh = m_unknown4c;
        g_piaSession->m_meshLayerController->MeshCleanup();
        if (createMesh)
            return SetNextStepAtFailedCreateMesh(cResultJoinFailed);
        return SetNextStepAtFailedJoinMesh(cResultJoinFailed);
    }
    if (m_unknown58 && !m_unknown5a)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        return SetNextStepAtFailedJoinMesh(cResultJoinFailed);
    }

    m_unknown58 = false;
    m_unknown56 = false;
    m_unknown55 = false;
    m_unknown59 = false;
    const Result result = g_piaSession->m_meshLayerController->MeshStartup(
        g_piaSession->m_matchmakeSession[g_piaSession->m_currentSessionIndex], false);
    if (result == 0)
    {
        g_piaSession->m_postponeNotice = true;
        SetNextStepAtMeshStartup();
        return cExecuteResultContinue;
    }

    const bool createMesh = m_unknown4c;
    g_piaSession->m_meshLayerController->MeshCleanup();
    if (createMesh)
        return SetNextStepAtFailedCreateMesh(result);
    return SetNextStepAtFailedJoinMesh(result);
}

unsigned int RandomMatchmakeJob::CreateMesh()
{
    const Result result = g_piaMesh->CreateMeshAsync();
    if (result != 0)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        return SetNextStepAtFailedCreateMesh(result);
    }
    g_piaMesh->SetSkipReport(true);
    setStep(&RandomMatchmakeJob::WaitCreateMesh, "RandomMatchmakeJob::WaitCreateMesh");
    return cExecuteResultNextStep;
}

unsigned int RandomMatchmakeJob::WaitCreateMesh()
{
    if (FailForSessionStatus(*this))
        return cExecuteResultContinue;

    if (!g_piaMesh->IsCreateMeshCompleted())
        return cExecuteResultNextStep;

    const Result result = g_piaMesh->GetCreateMeshResult();
    if (result == 0)
    {
        g_piaSession->m_matchmakeStatus = Session::cMatchmakeStatusMatched;
        g_piaSession->m_disconnectStatus = Session::cConnectionStatusConnected;
        if (g_piaSession->UpdateStationIdToBecomeHost())
        {
            g_piaSession->m_postponeNotice = false;
            g_piaSession->m_stationIdStatusTable->CallJoinEventNotYetNotified();
            SetNextStepAtWaitCreateMesh();
            return cExecuteResultContinue;
        }
        g_piaSession->m_meshLayerController->MeshCleanup();
        return SetNextStepAtFailedCreateMesh(cResultCreateFailed);
    }
    g_piaSession->m_meshLayerController->MeshCleanup();
    return SetNextStepAtFailedCreateMesh(result);
}

unsigned int RandomMatchmakeJob::GetStationConnection()
{
    if (FailForSessionStatus(*this))
        return cExecuteResultContinue;

    const unsigned int index = g_piaSession->m_currentSessionIndex;
    IMatchmakeSession* matchmakeSession = g_piaSession->m_matchmakeSession[index];
    const unsigned int sessionId = g_piaSession->m_sessionId[index];
    const transport::StationConnectionInfo emptyInfo;
    m_stationConnectionInfo.SetStationConnectionInfo(emptyInfo);
    const Result result = matchmakeSession->GetStationConnectionInfo(&m_callContext, sessionId);
    if (result == 0)
    {
        setStep(&RandomMatchmakeJob::WaitGetStationConnection, "RandomMatchmakeJob::WaitGetStationConnection");
        return cExecuteResultNextStep;
    }
    g_piaSession->m_meshLayerController->MeshCleanup();
    return SetNextStepAtFailedJoinMesh(result);
}

unsigned int RandomMatchmakeJob::WaitGetStationConnection()
{
    if (FailForSessionStatus(*this))
        return cExecuteResultContinue;

    IMatchmakeSession* matchmakeSession =
        g_piaSession->m_matchmakeSession[g_piaSession->m_currentSessionIndex];
    if (matchmakeSession->WaitGetStationConnectionInfo(&m_stationConnectionInfo))
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            g_piaSession->m_meshLayerController->MeshCleanup();
            return SetNextStepAtFailedJoinMesh(m_callContext.m_result);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            m_principalId = m_stationConnectionInfo.m_stationLocation.m_principalId;
            setStep(&RandomMatchmakeJob::JoinMesh, "RandomMatchmakeJob::JoinMesh");
            return cExecuteResultContinue;
        }
    }
    return cExecuteResultNextStep;
}

unsigned int RandomMatchmakeJob::JoinMesh()
{
    if (FailForSessionStatus(*this))
        return cExecuteResultContinue;

    const Result result = g_piaMesh->JoinMeshAsync(m_stationConnectionInfo);
    if (result == 0)
    {
        g_piaMesh->SetSkipReport(true);
        m_joinState = cJoinStateWaitJoinMesh;
        setStep(&RandomMatchmakeJob::WaitJoinMesh, "RandomMatchmakeJob::WaitJoinMesh");
        return cExecuteResultNextStep;
    }
    common::g_sessionBeginMonitoringContent.m_joinMeshState =
        static_cast<unsigned char>(g_piaSession->GetJoinMeshState());
    g_piaSession->m_meshLayerController->MeshCleanup();
    return SetNextStepAtFailedJoinMesh(result);
}

bool RandomMatchmakeJob::CheckContextCallCancelled()
{
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
        return true;
    return false;
}

unsigned int RandomMatchmakeJob::WaitJoinMesh()
{
    if (FailForSessionStatus(*this))
        return cExecuteResultContinue;

    if (!g_piaMesh->IsJoinMeshCompleted())
    {
        if (!m_unknown57 && !m_unknown58 && !m_unknown55 && !m_unknown56 && !m_unknown59 &&
            !CheckContextCallCancelled())
        {
            if (g_piaMesh->IsHostMigrationEnabled() &&
                (g_piaMesh->IsHostMigrationStarted() || g_piaMesh->IsProcessingHostMigration()) &&
                g_piaMesh->GetJoinMeshState() != cJoinMeshStateUnknown3)
            {
                m_unknown58 = true;
                g_piaMesh->CancelJoinMeshAsync();
            }
        }
        else
        {
            g_piaMesh->CancelJoinMeshAsync();
        }
        return cExecuteResultNextStep;
    }

    Result result = g_piaMesh->GetJoinMeshResult();
    if (result == 0)
    {
        const Result connectionError = g_piaMesh->CheckConnectionError();
        if (connectionError != 0)
        {
            m_result = cResultJoinFailed;
            g_piaSession->m_meshLayerController->MeshCleanup();
            return SetNextStepAtFailedJoinMesh(m_result);
        }
        SetNextStepAtWaitJoinMesh();
        return cExecuteResultContinue;
    }
    if (result == cResultCancelled &&
        (m_unknown57 || m_unknown58 || m_unknown55 || m_unknown56 || m_unknown59))
        result = cResultJoinFailed;
    g_piaSession->m_meshLayerController->MeshCleanup();
    return SetNextStepAtFailedJoinMesh(result);
}

unsigned int RandomMatchmakeJob::LeaveMesh()
{
    Result result;
    if (!g_piaMesh->IsHost())
        result = g_piaMesh->LeaveMesh(&m_callContext);
    else if (!g_piaMesh->IsHostMigrationEnabled())
        result = g_piaMesh->DestroyMesh(&m_callContext);
    else
        result = g_piaMesh->LeaveMeshWithHostMigration(&m_callContext);
    if (result != 0)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        return SetNextStepAtWaitLeaveMesh();
    }
    setStep(&RandomMatchmakeJob::WaitLeaveMesh, "RandomMatchmakeJob::WaitLeaveMesh");
    return cExecuteResultNextStep;
}

unsigned int RandomMatchmakeJob::WaitLeaveMesh()
{
    if (m_callContext.m_state != common::CallContext::cStateInProgress)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        return SetNextStepAtWaitLeaveMesh();
    }
    return cExecuteResultNextStep;
}

void RandomMatchmakeJob::CalcProcessingTimeOfJoin() const
{
    const int elapsedMsec = static_cast<int>(
        static_cast<long long>(common::g_piaJobScheduler->m_dispatchTime.m_tick - m_startTime.m_tick) /
        common::TimeSpan::GetTicksPerMSec().m_tick);
    if (elapsedMsec >= 0)
        common::g_sessionBeginMonitoringContent.m_joinProcessingTime = elapsedMsec;
}

unsigned int RandomMatchmakeJob::CompleteProcess()
{
    if (CheckContextCallCancelled())
    {
        setStep(&RandomMatchmakeJob::LeaveMesh, "RandomMatchmakeJob::LeaveMesh");
        m_result = cResultCancelled;
        return cExecuteResultContinue;
    }
    if (m_unknown57 || m_unknown58 || m_unknown55 ||
        (g_piaMesh->IsHostMigrationEnabled() &&
         (g_piaMesh->IsHostMigrationStarted() || g_piaMesh->IsProcessingHostMigration())))
    {
        setStep(&RandomMatchmakeJob::LeaveMesh, "RandomMatchmakeJob::LeaveMesh");
        m_result = cResultJoinFailed;
        return cExecuteResultContinue;
    }
    if (g_piaMesh->GetMeshState() == 0)
    {
        m_result = cResultJoinFailed;
        g_piaSession->m_meshLayerController->MeshCleanup();
        return SetNextStepAtWaitLeaveMesh();
    }

    m_unknown4d = true;
    g_piaSession->m_matchmakeStatus = Session::cMatchmakeStatusMatched;
    transport::g_piaTransport->m_stationIdTable->SetAvailableEntryNum(
        g_piaSession->m_fieldc2[g_piaSession->m_currentSessionIndex]);
    g_piaSession->m_disconnectStatus = Session::cConnectionStatusConnected;
    if (!g_piaSession->UpdateStationIdToBecomeClient())
    {
        m_result = cResultJoinFailed;
        common::g_sessionBeginMonitoringContent.m_joinMeshState =
            static_cast<unsigned char>(g_piaSession->GetJoinMeshState());
        setStep(&RandomMatchmakeJob::LeaveMesh, "RandomMatchmakeJob::LeaveMesh");
        return cExecuteResultContinue;
    }
    g_piaSession->m_postponeNotice = false;
    g_piaSession->m_stationIdStatusTable->ClearDisconnectedStationId();
    g_piaSession->m_stationIdStatusTable->CallJoinEventNotYetNotified();
    CalcProcessingTimeOfJoin();
    common::g_sessionBeginMonitoringContent.m_joinMeshState = cMonitoringByteUnset;
    if (!m_unknown4c)
        common::g_sessionBeginMonitoringContent.m_joinResult = 0;
    else
        common::g_sessionBeginMonitoringContent.m_joinResult = 0xffffffff;
    common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = 0xffffffff;
    g_piaMesh->SetSkipReport(m_unknown4c);
    g_piaMesh->SendReport(0);
    m_joinState = cJoinStateComplete;
    m_callerContext->SignalSuccess(0);
    m_callerContext = 0;
    return cExecuteResultFinished;
}

unsigned int RandomMatchmakeJob::CompleteFailure()
{
    g_piaSession->m_meshLayerController->MeshCleanup();
    if (!CheckContextCallCancelled() && m_result != cResultCancelled)
    {
        if (m_result == cResultCreateFailed || m_result == cResultSessionDisconnected)
        {
            g_piaSession->ChangeStatusToDisconnectedNetwork();
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_result = cResultSessionDisconnected;
        }
        else
        {
            m_callerContext->SignalFailure(m_result);
        }
    }
    else
    {
        m_callerContext->SignalCancel();
        m_result = cResultCancelled;
    }
    m_callerContext = 0;
    CalcProcessingTimeOfJoin();
    common::g_sessionBeginMonitoringContent.m_metric130 =
        static_cast<unsigned char>(g_piaSession->GetJoinRandomSessionStateForDebug());
    common::g_sessionBeginMonitoringContent.m_joinMeshState =
        static_cast<unsigned char>(g_piaSession->GetJoinMeshState());
    common::g_sessionBeginMonitoringContent.m_joinResult = m_result;
    m_joinState = cJoinStateComplete;
    g_piaMesh->SetSkipReport(false);
    g_piaMesh->SendReport(0);
    common::g_sessionBeginMonitoringContent.Cleanup();
    common::g_sessionStateMonitoringContent.Cleanup();
    Cleanup();
    return cExecuteResultFinished;
}

void RandomMatchmakeJob::SetNextStepAtMeshStartup()
{
    if (!m_unknown4c)
    {
        setStep(&RandomMatchmakeJob::GetStationConnection, "RandomMatchmakeJob::GetStationConnection");
    }
    else
    {
        transport::g_piaTransport->m_stationIdTable->SetAvailableEntryNum(
            g_piaSession->m_fieldc2[g_piaSession->m_currentSessionIndex]);
        setStep(&RandomMatchmakeJob::CreateMesh, "RandomMatchmakeJob::CreateMesh");
    }
}

void RandomMatchmakeJob::Cleanup()
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
    m_joinState = 0;
    m_unknown59 = false;
    m_principalId = 0;
    m_unknown54 = false;
    m_result = 0;
    m_unknown4d = false;
    m_unknown56 = false;
    m_unknown57 = false;
    m_unknown4c = false;
    m_unknown48 = 0;
    m_unknown55 = false;
    m_unknown58 = false;
    const transport::StationConnectionInfo emptyInfo;
    m_stationConnectionInfo.SetStationConnectionInfo(emptyInfo);
}

void RandomMatchmakeJob::CleanupInternalContexts()
{
    m_callContext.SignalCancel();
    m_callContext.Reset();
}

void RandomMatchmakeJob::Trace(unsigned long long value) const
{
}

void RandomMatchmakeJob::SetMatchmakeStatus(Session::MatchmakeStatus status)
{
    g_piaSession->m_matchmakeStatus = status;
}

void RandomMatchmakeJob::SetConnectionStatus(Session::ConnectionStatus status)
{
    g_piaSession->m_disconnectStatus = status;
}

unsigned int RandomMatchmakeJob::GetJoinSessionStateForDebug() const
{
    return m_joinState;
}

void RandomMatchmakeJob::NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId)
{
}

void RandomMatchmakeJob::NoticeUnregisterEvent(unsigned int sessionId)
{
}

void RandomMatchmakeJob::CleanupImpl()
{
}

}}}
