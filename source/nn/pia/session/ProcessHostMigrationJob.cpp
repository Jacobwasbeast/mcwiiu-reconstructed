#include "nn/pia/session/ProcessHostMigrationJob.hpp"

#include <coreinit/memory.h>

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/session/ProcessUpdateMeshJob.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"

namespace
{
unsigned int UpdateHostStationConnectionInfo(nn::pia::transport::Station* station)
{
    nn::pia::transport::StationConnectionInfoTable* table = nn::pia::transport::g_stationConnectionInfoTable;
    nn::pia::transport::StationConnectionInfo connectionInfo;
    if (table->GetStationConnectionInfo(station, &connectionInfo) != 0)
        return 0;
    table->m_hostStationConnectionInfo = connectionInfo;
    table->m_hostStationAddress = station->m_stationAddress;
    return 1;
}
}

namespace nn { namespace pia { namespace session {

namespace
{
enum
{
    cResultConnectionError6c50 = 0x6c50,
    cResultInvalidState = 0x10c08,
    cResultInvalidParameter = 0x10c07,
    cMeshEventKind2 = 2,
    cMeshEventKind3 = 3,
    cDisconnectReason7 = 7,
    cStationState6 = 6,
    cStationCountUnset = 0xff,
    cWaitMarginMsec = 500,
    cSequenceSendGreeting = 7,
    cSequenceWaitGreetingResponse = 8,
    cSequenceSendMigrationFinish = 10,
    cSequenceWaitNewHostGreeting = 11,
    cSequenceWaitNewHostFinished = 12,
    cTraceStation = 0x40000000,
    cGreetingResendIntervalMsec = 500,
    cBitmapTopBit = 0x80000000,
    cRankDecisionValue1 = 1,
    cRankDecisionValue2 = 2,
    cStationLocationField23Value2 = 2,
    cScoreWeightEdm = 0x800000,
    cScoreWeightNoRelay = 0x100,
    cScoreWeightRelayCount = 0x1000,
    cScoreWeightCallback = 0x1000000,
    cScoreUnset = 0xffffffff
};

const char* const cStepNameHostMigrationFailure = "ProcessHostMigrationJob::HostMigrationFailure";
const char* const cStepNameWaitGreetingResponse = "ProcessHostMigrationJob::WaitGreetingResponse";
const char* const cStepNameWaitNewHostFinished = "ProcessHostMigrationJob::WaitNewHostFinished";
const char* const cStepNameHostMigrationSuccess = "ProcessHostMigrationJob::HostMigrationSuccess";
}

ProcessHostMigrationJob::ProcessHostMigrationJob()
    : common::StepSequenceJob(), m_waitDeadline(), m_waitTimeMsec(10000), m_greetingDeadline(),
      m_startTime(), m_elapsedTime(), m_hostMigrationCount(0), m_stationCountAtStart(0),
      m_stationCountAtEnd(0), m_started(false), m_restartRequested(false)
{
    OSBlockSet(m_greetingPending, 0, sizeof(m_greetingPending));
    m_greetingSent = false;
    m_finishReceived = false;
    m_finishSuccess = false;
    m_unknownA5 = false;
    m_greetingSenderIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_oldHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_newHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_oldHostPrincipalId = 0;
    m_greetingSenderPrincipalId = 0;
    m_unknownB8 = false;
    m_unknownE4 = 0;
    m_unknownE0 = 0;
    m_unknownBC = g_piaMesh->m_hostMigrationMode == 2;
    OSBlockSet(m_rankDecision, 0, sizeof(m_rankDecision));
    OSBlockSet(m_candidateRanking, cStationIndexInvalid, sizeof(m_candidateRanking));
}

ProcessHostMigrationJob::~ProcessHostMigrationJob()
{
}

void ProcessHostMigrationJob::Trace(unsigned long long) const
{
}

unsigned int ProcessHostMigrationJob::IsFatalErrorOccur()
{
    return 0;
}

void ProcessHostMigrationJob::CheckWhetherNewHostIsUnknown(unsigned long long)
{
}

void ProcessHostMigrationJob::CleanupImpl()
{
}

unsigned int ProcessHostMigrationJob::StartupMultiImpl(bool, unsigned short)
{
    return 0;
}

unsigned int ProcessHostMigrationJob::CheckWhetherReselectNewHost()
{
    return 0;
}

void ProcessHostMigrationJob::ProcessingAtEndHM(bool stationCountValid)
{
    common::Time now;
    now.SetNow();
    m_elapsedTime.m_tick = now.m_tick - m_startTime.m_tick;
    if (stationCountValid)
        m_stationCountAtEnd = static_cast<unsigned char>(g_piaMesh->GetStationNum());
}

void ProcessHostMigrationJob::ProcessingAtStartHM()
{
    common::Time now;
    now.SetNow();
    m_startTime = now;
    m_elapsedTime.m_tick = 0;
    const unsigned char stationNum = static_cast<unsigned char>(g_piaMesh->GetStationNum());
    m_unknown6e = 0;
    m_stationCountAtStart = stationNum;
    ++m_hostMigrationCount;
    m_stationCountAtEnd = cStationCountUnset;
}

void ProcessHostMigrationJob::SearchRefugeRelayRoute()
{
    transport::RelayRouteManager* manager = transport::g_piaTransport->m_relayRouteManager;
    if (manager != 0)
    {
        const StationIndex host = g_piaMesh->GetHostStationIndex();
        if (host <= cStationIndexMax)
            manager->SearchRefugeRelayRoute(host);
    }
}

void ProcessHostMigrationJob::SetOldHostStation(StationIndex station, unsigned long long principalId)
{
    m_oldHostIndex = station;
    m_oldHostPrincipalId = principalId;
    if (principalId == 0)
    {
        transport::Station* oldHost = transport::g_piaStationManager->GetStation(m_oldHostIndex);
        if (oldHost != 0)
        {
            unsigned long long oldHostPrincipalId;
            if (oldHost->GetPrincipalId(&oldHostPrincipalId) == 0)
                m_oldHostPrincipalId = oldHostPrincipalId;
        }
    }
}

unsigned int ProcessHostMigrationJob::DecideNextHostCommonProc()
{
    SetOldHostStation(g_piaMesh->GetHostStationIndex(), 0);
    if (g_piaMesh->GetLocalStationIndex() <= cStationIndexMax && m_newHostIndex <= cStationIndexMax)
    {
        if (m_newHostIndex == g_piaMesh->GetLocalStationIndex())
        {
            const int waitMsec = m_waitTimeMsec - cWaitMarginMsec;
            const common::TimeSpan& ticksPerMSec = common::TimeSpan::GetTicksPerMSec();
            m_waitDeadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick + ticksPerMSec.m_tick * waitMsec;
            return 1;
        }
        m_unknownA5 = true;
        m_greetingSenderIndex = static_cast<StationIndex>(cStationIndexInvalid);
        m_greetingSenderPrincipalId = 0;
        const int waitMsec = m_waitTimeMsec;
        const common::TimeSpan& ticksPerMSec = common::TimeSpan::GetTicksPerMSec();
        m_waitDeadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick + ticksPerMSec.m_tick * waitMsec;
        return 2;
    }
    return 0;
}

unsigned int ProcessHostMigrationJob::CleanupOldHostInfoCommonProc()
{
    g_piaMesh->GetMeshProtocol()->SendMigrationResponse(m_oldHostIndex);
    DisconnectStation(m_oldHostIndex);
    return 1;
}

ProcessHostMigrationJob::ExecuteResult ProcessHostMigrationJob::SendGreetingMessage()
{
    m_hostMigrationSequence = cSequenceSendGreeting;
    if (g_piaMesh->CheckConnectionError() == cResultConnectionError6c50)
    {
        setStep(&ProcessHostMigrationJob::HostMigrationFailure, cStepNameHostMigrationFailure);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const StationIndex localIndex = g_piaMesh->GetLocalStationIndex();
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation == 0)
    {
        setStep(&ProcessHostMigrationJob::HostMigrationFailure, cStepNameHostMigrationFailure);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (!UpdateHostStationConnectionInfo(localStation))
    {
        localStation->Trace(cTraceStation);
        setStep(&ProcessHostMigrationJob::HostMigrationFailure, cStepNameHostMigrationFailure);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    transport::Station* oldHost = transport::g_piaStationManager->GetStation(m_oldHostIndex);
    if (oldHost != 0)
    {
        if (g_piaMesh->IsValidStationIndex(m_oldHostIndex))
            DisconnectStation(m_oldHostIndex);
        else
            oldHost->m_field20 = cStationState6;
    }
    int failedStations[cStationNumMax] = { 0 };
    int failedCount = 0;
    for (int index = 0; index < cStationNumMax; ++index)
    {
        if (index == localIndex || !g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(index)))
        {
            m_greetingPending[index] = 0;
        }
        else
        {
            const bool sent = g_piaMesh->GetMeshProtocol()->SendGreeting(static_cast<StationIndex>(index));
            m_greetingPending[index] = 1;
            if (!sent && transport::g_piaStationManager->GetStation(static_cast<StationIndex>(index)) == 0)
            {
                failedStations[failedCount] = index;
                ++failedCount;
                m_greetingPending[index] = 0;
            }
        }
    }
    for (int failed = 0; failed < failedCount; ++failed)
    {
        const StationIndex failedIndex = static_cast<StationIndex>(failedStations[failed]);
        g_piaMesh->UnfixDisconnectedId(failedIndex);
        transport::Station* station = transport::g_piaStationManager->GetStation(failedIndex);
        if (station != 0)
        {
            station->m_disconnectStationJob->ProcessDisconnection(station);
            transport::g_stationConnectionInfoTable->EraseFromTable(station);
            transport::IdentificationInfoTable::GetInstance()->EraseFromTable(station);
            station->CleanupJobs();
            station->Cleanup();
            transport::g_piaStationManager->DestroyStation(station);
            transport::g_piaTransport->OutputStreamUpdateEvent();
        }
    }
    m_greetingDeadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick +
        common::TimeSpan::GetTicksPerMSec().m_tick * cGreetingResendIntervalMsec;
    m_greetingSent = true;
    setStep(&ProcessHostMigrationJob::WaitGreetingResponse, cStepNameWaitGreetingResponse);
    return MakeExecuteResult(cExecuteKindNextStep);
}

ProcessHostMigrationJob::ExecuteResult ProcessHostMigrationJob::SendMigrationFinish()
{
    m_hostMigrationSequence = cSequenceSendMigrationFinish;
    if (g_piaMesh->CheckConnectionError() == cResultConnectionError6c50)
    {
        setStep(&ProcessHostMigrationJob::HostMigrationFailure, cStepNameHostMigrationFailure);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    g_piaMesh->GetMeshProtocol()->SendMigrationFinish(m_unknownBB);
    m_greetingSent = false;
    g_piaMesh->GetMeshProtocol()->SendUpdateMesh(true, 1);
    setStep(&ProcessHostMigrationJob::HostMigrationSuccess, cStepNameHostMigrationSuccess);
    return MakeExecuteResult(cExecuteKindContinue);
}

ProcessHostMigrationJob::ExecuteResult ProcessHostMigrationJob::WaitGreetingResponse()
{
    m_hostMigrationSequence = cSequenceWaitGreetingResponse;
    if (g_piaMesh->CheckConnectionError() == cResultConnectionError6c50)
    {
        setStep(&ProcessHostMigrationJob::HostMigrationFailure, cStepNameHostMigrationFailure);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    for (int index = 0; index < cStationNumMax; ++index)
    {
        if (m_greetingPending[index] != 0)
        {
            if (common::g_piaJobScheduler->m_dispatchTime < m_waitDeadline)
            {
                const common::Time now = common::g_piaJobScheduler->m_dispatchTime;
                if (m_greetingDeadline < now)
                {
                    for (int pending = 0; pending < cStationNumMax; ++pending)
                    {
                        if (m_greetingPending[pending] != 0)
                        {
                            if (!g_piaMesh->GetMeshProtocol()->SendGreeting(static_cast<StationIndex>(pending)))
                            {
                                if (!g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(pending)))
                                {
                                    m_greetingPending[pending] = 0;
                                }
                                else if (transport::g_piaStationManager->GetStation(static_cast<StationIndex>(pending)) == 0)
                                {
                                    g_piaMesh->UnfixDisconnectedId(static_cast<StationIndex>(pending));
                                    m_greetingPending[pending] = 0;
                                }
                            }
                            else
                            {
                                transport::g_piaStationManager->GetStation(static_cast<StationIndex>(pending));
                            }
                        }
                    }
                    m_greetingDeadline.m_tick = now.m_tick +
                        common::TimeSpan::GetTicksPerMSec().m_tick * cGreetingResendIntervalMsec;
                }
                return MakeExecuteResult(cExecuteKindNextStep);
            }
            m_unknownBB = false;
            PreprocessForSendingMigrationFinish();
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    m_unknownBB = true;
    PreprocessForSendingMigrationFinish();
    return MakeExecuteResult(cExecuteKindContinue);
}

ProcessHostMigrationJob::ExecuteResult ProcessHostMigrationJob::WaitNewHostGreeting()
{
    m_hostMigrationSequence = cSequenceWaitNewHostGreeting;
    if (g_piaMesh->CheckConnectionError() == cResultConnectionError6c50)
    {
        setStep(&ProcessHostMigrationJob::HostMigrationFailure, cStepNameHostMigrationFailure);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_greetingSenderIndex == cStationIndexInvalid)
    {
        if (m_waitDeadline <= common::g_piaJobScheduler->m_dispatchTime)
        {
            if (CheckWhetherReselectNewHost() == 0)
                setStep(&ProcessHostMigrationJob::HostMigrationFailure, cStepNameHostMigrationFailure);
            return MakeExecuteResult(cExecuteKindContinue);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    transport::Station* newHost;
    if (!m_unknownBC)
    {
        if (m_greetingSenderIndex != m_newHostIndex)
        {
            setStep(&ProcessHostMigrationJob::HostMigrationFailure, cStepNameHostMigrationFailure);
            return MakeExecuteResult(cExecuteKindContinue);
        }
        g_piaMesh->m_unknown58 = m_newHostIndex;
        transport::g_piaStationManager->m_invalidStationIndex = m_newHostIndex;
        newHost = transport::g_piaStationManager->GetStation(m_newHostIndex);
    }
    else
    {
        m_newHostIndex = m_greetingSenderIndex;
        g_piaMesh->m_unknown58 = m_greetingSenderIndex;
        transport::g_piaStationManager->m_invalidStationIndex = m_newHostIndex;
        newHost = transport::g_piaStationManager->GetStation(m_newHostIndex);
    }
    if (newHost == 0)
    {
        setStep(&ProcessHostMigrationJob::HostMigrationFailure, cStepNameHostMigrationFailure);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (!UpdateHostStationConnectionInfo(newHost))
    {
        newHost->Trace(cTraceStation);
        setStep(&ProcessHostMigrationJob::HostMigrationFailure, cStepNameHostMigrationFailure);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_oldHostIndex != m_newHostIndex)
    {
        if (transport::g_piaStationManager->GetStation(m_oldHostIndex) != 0)
            DisconnectStation(m_oldHostIndex);
        g_piaMesh->NoticeMeshEvent(static_cast<Mesh::EventType>(cMeshEventKind2), m_newHostIndex);
    }
    g_piaMesh->GetMeshProtocol()->SendGreetingResponse(m_newHostIndex);
    m_unknownA5 = false;
    m_unknownB8 = true;
    setStep(&ProcessHostMigrationJob::WaitNewHostFinished, cStepNameWaitNewHostFinished);
    return MakeExecuteResult(cExecuteKindNextStep);
}

ProcessHostMigrationJob::ExecuteResult ProcessHostMigrationJob::WaitNewHostFinished()
{
    m_hostMigrationSequence = cSequenceWaitNewHostFinished;
    if (g_piaMesh->CheckConnectionError() == cResultConnectionError6c50)
    {
        setStep(&ProcessHostMigrationJob::HostMigrationFailure, cStepNameHostMigrationFailure);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (!m_finishReceived)
    {
        if (common::g_piaJobScheduler->m_dispatchTime < m_waitDeadline && !m_restartRequested &&
            g_piaMesh->IsValidStationIndex(m_newHostIndex))
            return MakeExecuteResult(cExecuteKindNextStep);
        if (CheckWhetherReselectNewHost() == 0)
            setStep(&ProcessHostMigrationJob::HostMigrationFailure, cStepNameHostMigrationFailure);
        m_restartRequested = false;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    setStep(&ProcessHostMigrationJob::HostMigrationSuccess, cStepNameHostMigrationSuccess);
    m_finishSuccess = false;
    return MakeExecuteResult(cExecuteKindContinue);
}

ProcessHostMigrationJob::ExecuteResult ProcessHostMigrationJob::HostMigrationSuccess()
{
    if (g_piaMesh->CheckConnectionError() == cResultConnectionError6c50)
    {
        setStep(&ProcessHostMigrationJob::HostMigrationFailure, cStepNameHostMigrationFailure);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaMesh->IsHost())
    {
        g_piaMesh->NoticeMeshEvent(static_cast<Mesh::EventType>(cMeshEventKind2),
                                   g_piaMesh->GetLocalStationIndex());
        g_piaMesh->m_unknown56 = 1;
    }
    ProcessingAtEndHM(true);
    CleanupStatus();
    g_piaMesh->ProcessingAtEndHM();
    return MakeExecuteResult(cExecuteKindFinished);
}

ProcessHostMigrationJob::ExecuteResult ProcessHostMigrationJob::HostMigrationFailure()
{
    g_piaMesh->NoticeMeshEvent(static_cast<Mesh::EventType>(cMeshEventKind3),
                               static_cast<StationIndex>(cStationIndexInvalid));
    ProcessingAtEndHM(false);
    g_piaMesh->m_disconnectReason = cDisconnectReason7;
    g_piaMesh->ProcessingAtMeshEnd(static_cast<Mesh::DisconnectReason>(cDisconnectReason7));
    g_piaMesh->CleanupStatus();
    CleanupStatus();
    return MakeExecuteResult(cExecuteKindFinished);
}

void ProcessHostMigrationJob::ReceiveGreetingResponse(StationIndex station)
{
    if (station > cStationIndexMax)
        return;
    m_greetingPending[station] = 0;
}

void ProcessHostMigrationJob::ReceiveGreeting(StationIndex station)
{
    m_greetingSenderIndex = station;
    transport::Station* sender = transport::g_piaStationManager->GetStation(station);
    if (sender != 0)
    {
        unsigned long long senderPrincipalId;
        if (sender->GetPrincipalId(&senderPrincipalId) == 0)
            m_greetingSenderPrincipalId = senderPrincipalId;
    }
}

void ProcessHostMigrationJob::ReceiveFinish(bool success)
{
    m_finishSuccess = success;
    m_finishReceived = true;
    m_unknownB8 = false;
}

void ProcessHostMigrationJob::Cleanup()
{
    CleanupStatus();
    CleanupImpl();
}

Result ProcessHostMigrationJob::MakeHostCandidateRanking(StationIndex station, unsigned char* ranking,
                                                         unsigned int* meshValue,
                                                         unsigned int* relayValue,
                                                         bool fromConnectionInfo)
{
    Mesh* mesh = g_piaMesh;
    transport::StationConnectionInfoTable* connectionInfoTable = transport::g_stationConnectionInfoTable;
    unsigned int scores[cStationNumMax] = { 0 };
    transport::RelayRouteManager* relayManager = transport::g_piaTransport->m_relayRouteManager;
    if (!fromConnectionInfo)
    {
        for (int index = 0; index < mesh->m_unknown54; ++index)
        {
            scores[index] = cScoreUnset;
            if (index != station && mesh->IsValidStationIndex(static_cast<StationIndex>(index)))
            {
                int relayCount = 0;
                bool direct = true;
                if (relayManager != 0)
                {
                    StationIndex relay;
                    if (relayManager->GetOriginalRelayRoute(static_cast<StationIndex>(index), station,
                                                            &relay) == 0)
                        direct = station == relay;
                    unsigned int directStations;
                    if (relayManager->GetOriginalDirectStationList(static_cast<StationIndex>(index),
                                                                   &directStations) == 0)
                    {
                        for (; directStations != 0; directStations <<= 1)
                        {
                            if ((directStations & cBitmapTopBit) != 0)
                                ++relayCount;
                        }
                    }
                    if (direct)
                        --relayCount;
                }
                unsigned char callbackValue = 0;
                if (mesh->m_hostCandidateRankingCallback != 0)
                    callbackValue = mesh->m_hostCandidateRankingCallback(static_cast<StationIndex>(index), false);
                unsigned int edmWeight = 0;
                if (mesh->m_processUpdateMeshJob->CheckEdmByStationIndex(static_cast<StationIndex>(index)))
                    edmWeight = cScoreWeightEdm;
                unsigned int routeWeight = cScoreWeightNoRelay;
                if (direct)
                    routeWeight = 0;
                scores[index] = edmWeight + routeWeight +
                    (mesh->m_unknown54 - relayCount) * cScoreWeightRelayCount +
                    callbackValue * cScoreWeightCallback + index + 1;
            }
        }
    }
    else
    {
        transport::StationConnectionInfo connectionInfo;
        for (int index = 0; index < mesh->m_unknown54; ++index)
        {
            scores[index] = cScoreUnset;
            transport::Station* candidate = 0;
            if (index != station && mesh->IsValidStationIndex(static_cast<StationIndex>(index)) &&
                (candidate = transport::g_piaStationManager->GetStation(static_cast<StationIndex>(index))) != 0 &&
                connectionInfoTable->GetStationConnectionInfo(candidate, &connectionInfo) == 0)
            {
                int relayCount = 0;
                bool direct = true;
                if (relayManager != 0)
                {
                    StationIndex relay;
                    if (relayManager->GetOriginalRelayRoute(static_cast<StationIndex>(index), station,
                                                            &relay) == 0)
                        direct = station == relay;
                    unsigned int directStations;
                    if (relayManager->GetOriginalDirectStationList(static_cast<StationIndex>(index),
                                                                   &directStations) == 0)
                    {
                        for (; directStations != 0; directStations <<= 1)
                        {
                            if ((directStations & cBitmapTopBit) != 0)
                                ++relayCount;
                        }
                    }
                    if (direct)
                        --relayCount;
                }
                unsigned char callbackValue = 0;
                if (mesh->m_hostCandidateRankingCallback != 0)
                    callbackValue = mesh->m_hostCandidateRankingCallback(static_cast<StationIndex>(index), true);
                unsigned int edmWeight = 0;
                if (connectionInfo.m_stationLocation.m_field23 == cStationLocationField23Value2)
                    edmWeight = cScoreWeightEdm;
                unsigned int routeWeight = cScoreWeightNoRelay;
                if (direct)
                    routeWeight = 0;
                scores[index] = edmWeight + routeWeight +
                    (mesh->m_unknown54 - relayCount) * cScoreWeightRelayCount +
                    callbackValue * cScoreWeightCallback + index + 1;
            }
        }
    }
    unsigned int previous = 0;
    unsigned short ranked = 0;
    while (ranked < mesh->m_unknown54)
    {
        unsigned int best = cScoreUnset;
        unsigned int bestIndex;
        for (unsigned short index = 0; index < mesh->m_unknown54; ++index)
        {
            if (previous < scores[index] && scores[index] < best)
            {
                bestIndex = index;
                best = scores[index];
            }
        }
        if (best == cScoreUnset)
            break;
        ranking[ranked] = static_cast<unsigned char>(bestIndex);
        ++ranked;
        previous = best;
    }
    for (int fill = ranked; fill < mesh->m_unknown54; ++fill)
        ranking[fill] = cStationIndexInvalid;
    *meshValue = mesh->m_unknown80;
    unsigned int relayUnknown = 0;
    if (relayManager != 0)
        relayUnknown = relayManager->m_unknown1c;
    *relayValue = relayUnknown;
    return Result(0);
}

Result ProcessHostMigrationJob::CheckHostCandidateRanking(StationIndex first, StationIndex second,
                                                          bool* firstIsHigher, unsigned int* valueE0,
                                                          unsigned int* valueE4) const
{
    if (m_unknownE0 == 0)
        return Result(cResultInvalidState);
    if (first == cStationIndexInvalid || second == cStationIndexInvalid || firstIsHigher == 0)
        return Result(cResultInvalidParameter);
    unsigned int firstPosition = cStationNumMax;
    unsigned int secondPosition = cStationNumMax;
    for (unsigned short index = 0; index < g_piaMesh->m_unknown54; ++index)
    {
        if (m_candidateRanking[index] == static_cast<unsigned char>(first))
            firstPosition = index;
        if (m_candidateRanking[index] == static_cast<unsigned char>(second))
            secondPosition = index;
    }
    if (firstPosition == cStationNumMax && secondPosition == cStationNumMax)
        return Result(cResultInvalidParameter);
    *firstIsHigher = static_cast<int>(firstPosition - secondPosition) < 0;
    *valueE0 = m_unknownE0;
    *valueE4 = m_unknownE4;
    return Result(0);
}

void ProcessHostMigrationJob::SetRankDecision(StationIndex station, bool value)
{
    if (static_cast<int>(g_piaMesh->m_unknown54) <= station)
        return;
    m_rankDecision[station] = value ? cRankDecisionValue2 : cRankDecisionValue1;
}

void ProcessHostMigrationJob::InitializeMonitoringDataVar()
{
    m_unknown6e = 0;
    m_stationCountAtStart = cStationCountUnset;
    m_elapsedTime.m_tick = 0;
    m_stationCountAtEnd = cStationCountUnset;
    m_hostMigrationCount = 0;
}

void ProcessHostMigrationJob::SetMonitoringData()
{
    common::g_sessionStateMonitoringContent.m_metric185 = static_cast<unsigned short>(m_hostMigrationCount);
    const unsigned int elapsedMsec = static_cast<unsigned int>(
        m_elapsedTime.m_tick / common::TimeSpan::GetTicksPerMSec().m_tick);
    common::g_sessionStateMonitoringContent.m_metric186 = elapsedMsec;
    if (elapsedMsec == 0)
        common::g_sessionStateMonitoringContent.m_metric186 = 0xffffffff;
    common::g_sessionStateMonitoringContent.m_metric187[0] = m_stationCountAtStart;
    common::g_sessionStateMonitoringContent.m_metric187[1] = m_stationCountAtEnd;
    common::g_sessionStateMonitoringContent.m_metric225[2] = m_unknown6e;
}

void ProcessHostMigrationJob::CleanupStatus()
{
    m_started = false;
    m_restartRequested = false;
    OSBlockSet(m_greetingPending, 0, sizeof(m_greetingPending));
    m_oldHostPrincipalId = 0;
    m_newHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_unknownE4 = 0;
    m_greetingSenderPrincipalId = 0;
    m_unknownB8 = false;
    m_unknownA5 = false;
    m_finishSuccess = false;
    m_oldHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_finishReceived = false;
    m_greetingSent = false;
    m_greetingSenderIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_unknownE0 = 0;
    OSBlockSet(m_rankDecision, 0, sizeof(m_rankDecision));
    m_hostMigrationSequence = 0;
}

void ProcessHostMigrationJob::DisconnectStation(StationIndex station)
{
    if (g_piaMesh->IsValidStationIndex(station))
    {
        g_piaMesh->UnfixDisconnectedId(station);
        transport::Station* target = transport::g_piaStationManager->GetStation(station);
        if (target != 0)
        {
            target->m_field20 = cStationState6;
            target->SetStationIdAndIndex(0, static_cast<StationIndex>(cStationIndexInvalid));
        }
    }
}

StationIndex ProcessHostMigrationJob::GetStationIndexByPrincipalID(unsigned long long principalId)
{
    return g_piaMesh->m_processUpdateMeshJob->GetStationIndexByPrincipalID(principalId);
}

}}}
