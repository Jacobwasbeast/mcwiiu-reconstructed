#include "nn/pia/session/JoinMeshJob.hpp"

#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/KickoutManageJob.hpp"
#include "nn/pia/session/LeaveMeshJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshEventListener.hpp"
#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/ProcessConnectionRequestJob.hpp"
#include "nn/pia/transport/ResendingMessageManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
enum JoinResult
{
    cResultCancelled = 0x6c05,
    cResultSetupLocalPlayerInfoFailed = 0x6c65,
    cResultJoinFailed = 0x6439,
    cResultJoinResponse0 = 0x6c3d,
    cResultJoinResponse1 = 0x6838,
    cResultJoinResponseUnknown = 0x10c3e,
    cResultConnectionRefused = 0xc24,
    cResultConnectionFailed10c25 = 0x10c25,
    cResultConnectionFailed643f = 0x643f,
    cResultConnectionFailed6442 = 0x6442,
    cResultConnectionFailed6443 = 0x6443,
    cResultConnectionFailed6444 = 0x6444,
    cResultConnectionFailed6445 = 0x6445,
    cResultConnectionFailed6467 = 0x6467,
    cResultConnectionFailed6468 = 0x6468,
    cResultConnectionFailed6469 = 0x6469,
    cResultConnectionFailed646a = 0x646a,
    cResultConnectionFailed644a = 0x644a
};

enum MeshEventKind
{
    cMeshEvent14 = 0x14,
    cMeshEvent16 = 0x16,
    cMeshEvent17 = 0x17
};

enum JoinResponseLayout
{
    cResponseCount = 1,
    cResponseHostSlot = 2,
    cResponseLocalSlot = 3,
    cResponsePartCount = 4,
    cResponsePartIndex = 5,
    cResponseEntryCount = 6,
    cResponseEntryStart = 7,
    cResponseEventIdHigh = 8,
    cResponseEventIdLow = 9,
    cResponseEventIdByte = 10,
    cResponseSerialNumber = 12,
    cResponseFirstPartEntries = 16,
    cResponseNextPartEntries = 8,
    cResponseSlotNone = 0xff
};

enum
{
    cStationIndexInvalid = 0xfd,
    cLocalAndHostStationCount = 2,
    cConnectionIdModulo = 0xfe,
    cConnectionIdBase = 2,
    cDisconnectTimeoutMsec = 5000,
    cConnectedStationState = 5,
    cConnectingStationState = 2,
    cTraceStationLevel = 0x4000000,
    cMonitoringByteUnset = 0xff
};
}

JoinMeshJob::JoinMeshJob()
    : common::StepSequenceJob(), m_callerContext(0), m_requestAckId(0), m_waitingJoinResponse(false),
      m_responseResult(3), m_lastUpdateCount(0), m_joinState(0), m_joinResult(0), m_startTime(),
      m_disconnectDeadline(), m_callContext()
{
    for (int index = 0; index < cReceivedFlagNum; ++index)
        m_received[index] = false;
    void* contextStorage = pead::allocateFromSeadHeap(sizeof(common::CallContext), common::HeapManager::GetHeap());
    m_internalContext = new (contextStorage) common::CallContext();
    m_stationCapacity = g_piaMesh->m_unknown54;
    unsigned int count = m_stationCapacity;
    transport::StationConnectionInfo* infos = static_cast<transport::StationConnectionInfo*>(
        pead::allocateFromSeadHeap(count * sizeof(transport::StationConnectionInfo), common::HeapManager::GetHeap()));
    if (infos != 0)
    {
        transport::StationConnectionInfo* info = infos;
        for (; count != 0; --count)
        {
            new (info) transport::StationConnectionInfo();
            ++info;
        }
    }
    m_stationInfos = infos;
    count = m_stationCapacity;
    unsigned int* indices = static_cast<unsigned int*>(
        pead::allocateFromSeadHeap(count * sizeof(unsigned int), common::HeapManager::GetHeap()));
    if (indices != 0)
    {
        unsigned int* index = indices;
        for (; count != 0; --count)
        {
            new (index) unsigned int();
            ++index;
        }
    }
    m_stationIndices = indices;
    count = m_stationCapacity;
    unsigned char* connected = static_cast<unsigned char*>(
        pead::allocateFromSeadHeap(count, common::HeapManager::GetHeap()));
    if (connected != 0)
    {
        unsigned char* flag = connected;
        for (; count != 0; --count)
        {
            new (flag) unsigned char();
            ++flag;
        }
    }
    m_stationConnected = connected;
    for (unsigned short index = 0; index < m_stationCapacity; ++index)
        m_stationConnected[index] = 0;
}

JoinMeshJob::~JoinMeshJob()
{
    CleanupInternalContexts();
    if (m_internalContext != 0)
    {
        m_internalContext->~CallContext();
        pead::freeToSeadHeap(m_internalContext);
    }
    if (m_stationInfos != 0)
    {
        unsigned int count = pead::getArraySize(m_stationInfos) / sizeof(transport::StationConnectionInfo);
        transport::StationConnectionInfo* info = m_stationInfos;
        for (; count != 0; --count)
        {
            info->~StationConnectionInfo();
            ++info;
        }
        pead::freeToSeadHeap(m_stationInfos);
    }
    if (m_stationIndices != 0)
    {
        pead::getArraySize(m_stationIndices);
        pead::freeToSeadHeap(m_stationIndices);
    }
    if (m_stationConnected != 0)
    {
        pead::getArraySize(m_stationConnected);
        pead::freeToSeadHeap(m_stationConnected);
    }
}

bool JoinMeshJob::Startup(const transport::StationConnectionInfo& info, common::CallContext* callerContext)
{
    if (callerContext != 0)
    {
        m_callerContext = callerContext;
        callerContext->InitiateCall();
    }
    if (StartupImpl() == 0)
        return false;
    *m_stationInfos = info;
    m_joinResult = 0;
    m_waitingJoinResponse = false;
    for (int index = 0; index < cReceivedFlagNum; ++index)
        m_received[index] = false;
    m_requestAckId = 0;
    m_lastUpdateCount = 0;
    for (unsigned int index = 0; index < m_stationCapacity; ++index)
        m_stationConnected[index] = 0;
    g_piaMesh->CleanupMonitoringBeginData();
    SetHostInfoToMonitoringData(info);
    m_principalId = info.m_stationLocation.m_principalId;
    m_startTime = common::g_piaJobScheduler->m_dispatchTime;
    return true;
}

JoinMeshJob::ExecuteResult JoinMeshJob::SetupLocalPlayerInfo()
{
    g_piaMesh->SetupLocalPlayerInfo();
    SetNextStepAtSetupLocalPlayerInfoImpl();
    return MakeExecuteResult(cExecuteKindEntryForeground);
}

JoinMeshJob::ExecuteResult JoinMeshJob::SetupLocalStation()
{
    g_piaMesh->SetupProtocols();
    transport::Station* station = transport::g_piaStationManager->CreateLocalStation();
    if (!station->Startup())
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess ||
            m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalFailure(cResultJoinFailed);
        common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        g_piaMesh->ProcessingAtMeshBegin();
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    g_piaMesh->ResetStationIndexUsage();
    station->m_stationAddress = transport::g_stationConnectionInfoTable->m_stationLocation.m_stationAddress;
    Result result = transport::g_stationConnectionInfoTable->AddToTable(station, *transport::g_stationConnectionInfoTable);
    if (result != 0)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess ||
            m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalFailure(cResultJoinFailed);
        common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        g_piaMesh->ProcessingAtMeshBegin();
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    setStep(&JoinMeshJob::StartConnectingToHost, "JoinMeshJob::StartConnectingToHost");
    return MakeExecuteResult(cExecuteKindContinue);
}

void JoinMeshJob::CleanupStations()
{
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation != 0)
    {
        transport::Station** station = transport::g_piaStationManager->m_stations.data;
        if (station != station + transport::g_piaStationManager->m_stations.size)
        {
            do
            {
                if (*station != localStation)
                {
                    if (g_piaMesh->IsValidStationIndex(static_cast<StationIndex>((*station)->m_field10)))
                        g_piaMesh->UnfixDisconnectedId(static_cast<StationIndex>((*station)->m_field10));
                    (*station)->m_connectStationJob->Cleanup();
                    (*station)->m_processConnectionRequestJob->Cleanup();
                }
                ++station;
            } while (station != transport::g_piaStationManager->m_stations.data + transport::g_piaStationManager->m_stations.size);
        }
    }
}

void JoinMeshJob::CalcProcessingTimeOfJoin() const
{
    const int elapsedMsec = static_cast<int>(
        static_cast<long long>(common::g_piaJobScheduler->m_dispatchTime.m_tick - m_startTime.m_tick) /
        common::TimeSpan::GetTicksPerMSec().m_tick);
    if (elapsedMsec >= 0)
    {
        common::g_sessionBeginMonitoringContent.m_joinMeshState = static_cast<unsigned char>(m_joinState);
        common::g_sessionBeginMonitoringContent.m_joinProcessingTime = elapsedMsec;
    }
}

void JoinMeshJob::CheckRelayConnectionForMonitoring() const
{
}

unsigned int JoinMeshJob::CheckContextCallCancelled()
{
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        CleanupInternalContexts();
        CleanupStations();
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        CalcProcessingTimeOfJoin();
        CheckRelayConnectionForMonitoring();
        common::g_sessionBeginMonitoringContent.m_joinResult = cResultCancelled;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        common::g_sessionBeginMonitoringContent.m_joinMeshState = static_cast<unsigned char>(m_joinState);
        g_piaMesh->ProcessingAtMeshBegin();
        return 1;
    }
    return 0;
}

JoinMeshJob::ExecuteResult JoinMeshJob::StartConnectingToHost()
{
    if (CheckContextCallCancelled() != 0)
        return MakeExecuteResult(cExecuteKindFinished);
    m_joinState = 1;
    transport::Station* station = transport::g_piaStationManager->CreateStation();
    if (station->Startup())
    {
        station->SetStationIdAndIndex(
            m_stationInfos->m_stationLocation.m_principalId,
            g_piaMesh->GetHostStationIndex());
        station->m_stationAddress = m_stationInfos->m_stationLocation.m_stationAddress;
        transport::g_stationConnectionInfoTable->AddToTable(station, *m_stationInfos);
        common::Time now;
        now.SetNow();
        station->m_field54 = static_cast<unsigned char>(now.m_tick % cConnectionIdModulo + cConnectionIdBase);
    }
    transport::ConnectStationJob* connectJob = station->m_connectStationJob;
    if (connectJob->GetState() != cStateNotEntried && connectJob->GetState() != cStateCompleted)
    {
        connectJob->Cleanup();
        connectJob->Reset(true);
    }
    station->m_field20 = cConnectingStationState;
    connectJob->Ready(false);
    setStep(&JoinMeshJob::WaitUntilConnectToHost, "JoinMeshJob::WaitUntilConnectToHost");
    return MakeExecuteResult(cExecuteKindNextStep);
}

unsigned int JoinMeshJob::CheckTransportConnectionStatus()
{
    if (transport::g_piaTransport->m_connectionError == 0)
        return 0;
    CalcProcessingTimeOfJoin();
    CheckRelayConnectionForMonitoring();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess ||
            m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalFailure(cResultJoinFailed);
        common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        g_piaMesh->ProcessingAtMeshBegin();
        m_callerContext = 0;
    }
    CleanupInternalContexts();
    return 1;
}

JoinMeshJob::ExecuteResult JoinMeshJob::WaitUntilConnectToHost()
{
    if (CheckContextCallCancelled() != 0)
        return MakeExecuteResult(cExecuteKindFinished);
    common::CallContext* context = m_internalContext;
    if (context->m_state < common::CallContext::cStateSuccess ||
        context->m_state > common::CallContext::cStateCancelled)
    {
        if (CheckTransportConnectionStatus() != 0)
            return MakeExecuteResult(cExecuteKindFinished);
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (context->m_result == 0)
    {
        transport::Station* station = transport::g_piaStationManager->GetStation(g_piaMesh->GetHostStationIndex());
        if (station == 0)
        {
            CalcProcessingTimeOfJoin();
            if (m_callerContext != 0)
            {
                if (m_callerContext->m_state < common::CallContext::cStateSuccess ||
                    m_callerContext->m_state > common::CallContext::cStateCancelled)
                    m_callerContext->SignalFailure(cResultJoinFailed);
                common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
                common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
                g_piaMesh->ProcessingAtMeshBegin();
                m_callerContext = 0;
            }
            CleanupInternalContexts();
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (!(transport::g_stationConnectionInfoTable->m_hostStationAddress == station->m_stationAddress))
        {
            station->m_stationAddress.Trace(cTraceStationLevel);
            transport::g_stationConnectionInfoTable->m_hostStationAddress.Trace(cTraceStationLevel);
        }
        transport::StationConnectionInfo hostInfo;
        transport::g_stationConnectionInfoTable->GetStationConnectionInfo(station, &hostInfo);
        transport::g_stationConnectionInfoTable->m_hostStationConnectionInfo = hostInfo;
        transport::g_stationConnectionInfoTable->m_hostStationAddress = station->m_stationAddress;
        setStep(&JoinMeshJob::SendJoinRequest, "JoinMeshJob::SendJoinRequest");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    Result result;
    bool notifyEvent = false;
    switch (context->m_result)
    {
    case cResultConnectionRefused:
        result = cResultJoinResponse0;
        break;
    case cResultConnectionFailed10c25:
        result = cResultConnectionFailed10c25;
        break;
    case cResultConnectionFailed643f:
    case cResultConnectionFailed6442:
    case cResultConnectionFailed6443:
    case cResultConnectionFailed6444:
    case cResultConnectionFailed6445:
    case cResultConnectionFailed6467:
    case cResultConnectionFailed6468:
    case cResultConnectionFailed6469:
    case cResultConnectionFailed646a:
    case cResultConnectionFailed644a:
        result = context->m_result;
        notifyEvent = true;
        break;
    default:
        result = cResultJoinFailed;
        break;
    }
    if (notifyEvent && g_piaMesh->m_meshEventListener != 0)
    {
        MeshEventListener::MeshEvent event;
        event.type = cMeshEvent16;
        event.stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
        event.stationId = 0;
        static_cast<MeshEventListener*>(g_piaMesh->m_meshEventListener)->NoticeEvent(event);
    }
    CalcProcessingTimeOfJoin();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess ||
            m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalFailure(result);
        common::g_sessionBeginMonitoringContent.m_joinResult = result;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        g_piaMesh->ProcessingAtMeshBegin();
        m_callerContext = 0;
    }
    CleanupInternalContexts();
    return MakeExecuteResult(cExecuteKindFinished);
}

JoinMeshJob::ExecuteResult JoinMeshJob::SendJoinRequest()
{
    if (CheckContextCallCancelled() != 0)
        return MakeExecuteResult(cExecuteKindFinished);
    CalcProcessingTimeOfJoin();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess ||
            m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalFailure(cResultJoinFailed);
        common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        g_piaMesh->ProcessingAtMeshBegin();
        m_callerContext = 0;
    }
    CleanupInternalContexts();
    return MakeExecuteResult(cExecuteKindFinished);
}

unsigned int JoinMeshJob::CheckConnectionStateWithHostStation()
{
    transport::Station* station = transport::g_piaStationManager->GetStation(g_piaMesh->GetHostStationIndex());
    if (station != 0 && station->m_field20 == cConnectedStationState)
        return 0;
    CalcProcessingTimeOfJoin();
    CheckRelayConnectionForMonitoring();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state < common::CallContext::cStateSuccess ||
            m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalFailure(cResultJoinFailed);
        common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        g_piaMesh->ProcessingAtMeshBegin();
        m_callerContext = 0;
    }
    CleanupInternalContexts();
    return 1;
}

JoinMeshJob::ExecuteResult JoinMeshJob::WaitRequestAck()
{
    if (!transport::g_piaResendingMessageManager->CheckNowResending(m_requestAckId))
    {
        m_requestAckId = 0;
        setStep(&JoinMeshJob::WaitJoinResponse, "JoinMeshJob::WaitJoinResponse");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (CheckContextCallCancelled() != 0)
    {
        transport::g_piaResendingMessageManager->StopResending(m_requestAckId);
        m_requestAckId = 0;
        m_waitingJoinResponse = false;
        for (int index = 0; index < cReceivedFlagNum; ++index)
            m_received[index] = false;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (CheckTransportConnectionStatus() == 0 && CheckConnectionStateWithHostStation() == 0)
    {
        if (!m_waitingJoinResponse)
        {
            transport::g_piaResendingMessageManager->StopResending(m_requestAckId);
            setStep(&JoinMeshJob::AnalyzeJoinResponse, "JoinMeshJob::AnalyzeJoinResponse");
            m_requestAckId = 0;
            return MakeExecuteResult(cExecuteKindContinue);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    transport::g_piaResendingMessageManager->StopResending(m_requestAckId);
    m_requestAckId = 0;
    m_waitingJoinResponse = false;
    for (int index = 0; index < cReceivedFlagNum; ++index)
        m_received[index] = false;
    return MakeExecuteResult(cExecuteKindFinished);
}

JoinMeshJob::ExecuteResult JoinMeshJob::WaitJoinResponse()
{
    if (!m_waitingJoinResponse)
    {
        setStep(&JoinMeshJob::AnalyzeJoinResponse, "JoinMeshJob::AnalyzeJoinResponse");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (CheckContextCallCancelled() != 0)
    {
        m_waitingJoinResponse = false;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (CheckTransportConnectionStatus() == 0 && CheckConnectionStateWithHostStation() == 0)
        return MakeExecuteResult(cExecuteKindNextStep);
    m_waitingJoinResponse = false;
    return MakeExecuteResult(cExecuteKindFinished);
}

unsigned int JoinMeshJob::UpdateLocalAndHostInformation(StationIndex localIndex, StationIndex hostIndex)
{
    transport::Station* hostStation = transport::g_piaStationManager->GetStation(g_piaMesh->GetHostStationIndex());
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (hostStation != 0 && localStation != 0)
    {
        localStation->SetStationIdAndIndex(
            transport::g_stationConnectionInfoTable->m_stationLocation.m_principalId,
            localIndex);
        g_piaMesh->m_unknown5c = localIndex;
        g_piaMesh->StartUse(localIndex);
        localStation->m_field20 = cConnectedStationState;
        hostStation->SetStationIdAndIndex(
            (static_cast<unsigned long long>(hostStation->m_field18) << 32) | static_cast<unsigned int>(hostStation->m_field1c),
            hostIndex);
        g_piaMesh->m_unknown58 = hostIndex;
        g_piaMesh->NoticeMeshEvent(Mesh::cEventTypeJoin, localIndex);
        g_piaMesh->FixConnectedId(hostIndex);
        transport::g_piaStationManager->m_invalidStationIndex = hostIndex;
        return 1;
    }
    return 0;
}

unsigned int JoinMeshJob::SetStationDataList(const unsigned char* data, unsigned int)
{
    const unsigned int infoSize = m_stationInfos->GetSerializedSize();
    const unsigned int entrySize = infoSize + (~infoSize & 3) + 1;
    const unsigned char hostSlot = data[cResponseHostSlot];
    const unsigned char count = data[cResponseCount];
    const unsigned char localSlot = data[cResponseLocalSlot];
    if (count == 0 || hostSlot == cResponseSlotNone || localSlot == cResponseSlotNone)
    {
        m_stationCount = 0;
        m_responseResult = data[cResponsePartCount];
        m_waitingJoinResponse = false;
        return 1;
    }
    if (m_stationCapacity < count)
    {
        m_stationCount = 0;
        m_responseResult = 3;
        m_waitingJoinResponse = false;
        return 1;
    }
    const unsigned char partCount = data[cResponsePartCount];
    if (partCount != 1)
    {
        if (partCount < 2 || partCount > 3)
            return 0;
        const unsigned char partIndex = data[cResponsePartIndex];
        if (!m_received[partIndex])
            return 1;
        unsigned int entryOffset = cResponseNextPartEntries;
        if (partIndex == 0)
        {
            if (g_piaMesh->m_meshEventListener != 0)
            {
                MeshEventListener::MeshEvent event;
                event.type = cMeshEvent17;
                event.stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
                event.stationId = (static_cast<unsigned int>(data[cResponseEventIdByte]) << 16) |
                    (static_cast<unsigned int>(data[cResponseEventIdHigh]) << 8) | data[cResponseEventIdLow];
                static_cast<MeshEventListener*>(g_piaMesh->m_meshEventListener)->NoticeEvent(event);
            }
            g_piaMesh->m_unknownc8 = common::deserializeU32(data + cResponseSerialNumber);
            entryOffset = cResponseFirstPartEntries;
        }
        bool firstPart = true;
        for (int index = 0; index < cReceivedFlagNum; ++index)
            firstPart = firstPart & m_received[index];
        if (!firstPart)
        {
            if (m_responseStationCount != count)
                return 0;
            if (m_responseHostSlot != hostSlot)
                return 0;
            if (m_responseLocalSlot != localSlot)
                return 0;
            if (m_responsePartCount != partCount)
                return 0;
        }
        else
        {
            m_responseHostSlot = hostSlot;
            m_responseLocalSlot = localSlot;
            m_responseStationCount = count;
            m_responsePartCount = partCount;
        }
        const unsigned char entryCount = data[cResponseEntryCount];
        const unsigned char entryStart = data[cResponseEntryStart];
        for (unsigned int entry = 0; entry < entryCount; ++entry)
        {
            (m_stationInfos + entryStart + entry)->Deserialize(data + entryOffset + entry * entrySize);
            m_stationIndices[entryStart + entry] = data[entryOffset + entry * entrySize + infoSize];
        }
        m_received[partIndex] = false;
        bool pending = false;
        for (unsigned char index = 0; index < m_responsePartCount; ++index)
            pending = pending | m_received[index];
        if (pending)
            return 1;
        const unsigned int hostIndex = m_stationIndices[m_responseHostSlot];
        const unsigned int localIndex = m_stationIndices[m_responseLocalSlot];
        transport::StationConnectionInfo receivedLocalInfo(m_stationInfos[m_responseLocalSlot]);
        transport::StationConnectionInfo localInfo;
        Result result = transport::g_piaTransport->GetLocalStationConnectionInfo(&localInfo);
        if (hostIndex < transport::StationManager::cStationNumMax &&
            localIndex < transport::StationManager::cStationNumMax && result == 0 &&
            receivedLocalInfo == localInfo)
        {
            if (UpdateLocalAndHostInformation(static_cast<StationIndex>(localIndex), static_cast<StationIndex>(hostIndex)) == 0)
                return 0;
            m_stationCount = m_responseStationCount;
            m_waitingJoinResponse = false;
            return 1;
        }
        localInfo.Trace(cTraceStationLevel);
        receivedLocalInfo.Trace(cTraceStationLevel);
        m_stationCount = 0;
        m_responseResult = 3;
        m_waitingJoinResponse = false;
        return 1;
    }
    if (g_piaMesh->m_meshEventListener != 0)
    {
        MeshEventListener::MeshEvent event;
        event.type = cMeshEvent17;
        event.stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
        event.stationId = (static_cast<unsigned int>(data[cResponseEventIdByte]) << 16) |
            (static_cast<unsigned int>(data[cResponseEventIdHigh]) << 8) | data[cResponseEventIdLow];
        static_cast<MeshEventListener*>(g_piaMesh->m_meshEventListener)->NoticeEvent(event);
    }
    g_piaMesh->m_unknownc8 = common::deserializeU32(data + cResponseSerialNumber);
    const unsigned char hostIndex = data[cResponseFirstPartEntries + entrySize * hostSlot + infoSize];
    const unsigned char localIndex = data[cResponseFirstPartEntries + entrySize * localSlot + infoSize];
    transport::StationConnectionInfo receivedLocalInfo;
    transport::StationConnectionInfo localInfo;
    receivedLocalInfo.Deserialize(data + cResponseFirstPartEntries + entrySize * localSlot);
    Result result = transport::g_piaTransport->GetLocalStationConnectionInfo(&localInfo);
    if (hostIndex < transport::StationManager::cStationNumMax &&
        localIndex < transport::StationManager::cStationNumMax && result == 0 &&
        receivedLocalInfo == localInfo)
    {
        if (UpdateLocalAndHostInformation(static_cast<StationIndex>(localIndex), static_cast<StationIndex>(hostIndex)) == 0)
            return 0;
        m_stationCount = count;
        for (unsigned int entry = 0; entry < m_stationCount; ++entry)
        {
            (m_stationInfos + entry)->Deserialize(data + cResponseFirstPartEntries + entry * entrySize);
            m_stationIndices[entry] = data[cResponseFirstPartEntries + entry * entrySize + infoSize];
        }
        m_waitingJoinResponse = false;
        return 1;
    }
    localInfo.Trace(cTraceStationLevel);
    receivedLocalInfo.Trace(cTraceStationLevel);
    m_stationCount = 0;
    m_responseResult = 3;
    m_waitingJoinResponse = false;
    return 1;
}

JoinMeshJob::ExecuteResult JoinMeshJob::AnalyzeJoinResponse()
{
    if (CheckContextCallCancelled() != 0)
        return MakeExecuteResult(cExecuteKindFinished);
    if (m_stationCount != 0)
    {
        common::g_sessionBeginMonitoringContent.m_metric47 = static_cast<unsigned char>(m_stationCount);
        m_principalId = 0xffffffffULL;
        setStep(&JoinMeshJob::WaitAllConnection, "JoinMeshJob::WaitAllConnection");
        m_joinState = 2;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    CalcProcessingTimeOfJoin();
    if (m_callerContext != 0)
    {
        Result result;
        switch (m_responseResult)
        {
        case 1:
            result = cResultJoinResponse1;
            break;
        case 0:
            result = cResultJoinResponse0;
            break;
        case 2:
            if (g_piaMesh->m_meshEventListener != 0)
            {
                MeshEventListener::MeshEvent event;
                event.type = cMeshEvent14;
                event.stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
                event.stationId = 0;
                static_cast<MeshEventListener*>(g_piaMesh->m_meshEventListener)->NoticeEvent(event);
            }
            result = cResultJoinFailed;
            break;
        case 4:
            result = cResultJoinFailed;
            break;
        default:
            result = cResultJoinResponseUnknown;
            break;
        }
        if (m_callerContext->m_state < common::CallContext::cStateSuccess ||
            m_callerContext->m_state > common::CallContext::cStateCancelled)
            m_callerContext->SignalFailure(result);
        common::g_sessionBeginMonitoringContent.m_joinResult = result;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        g_piaMesh->ProcessingAtMeshBegin();
        m_callerContext = 0;
    }
    CleanupInternalContexts();
    return MakeExecuteResult(cExecuteKindFinished);
}

JoinMeshJob::ExecuteResult JoinMeshJob::WaitAllConnection()
{
    if (CheckContextCallCancelled() != 0)
    {
        setStep(&JoinMeshJob::StartDisconnectStations, "JoinMeshJob::StartDisconnectStations");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaMesh->m_leaveMeshJob->IsRunning())
        return MakeExecuteResult(cExecuteKindNextStep);
    if (g_piaMesh->m_kickoutManageJob->IsRunning())
        return MakeExecuteResult(cExecuteKindNextStep);
    const int localStationIndex = g_piaMesh->GetLocalStationIndex();
    if (localStationIndex >= transport::StationManager::cStationNumMax)
    {
        CalcProcessingTimeOfJoin();
        if (m_callerContext != 0)
        {
            Result result;
            switch (m_joinResult)
            {
            case cResultConnectionFailed643f:
            case cResultConnectionFailed6442:
            case cResultConnectionFailed6443:
            case cResultConnectionFailed6444:
            case cResultConnectionFailed6445:
            case cResultConnectionFailed6467:
            case cResultConnectionFailed6468:
            case cResultConnectionFailed6469:
            case cResultConnectionFailed646a:
            case cResultConnectionFailed644a:
                result = m_joinResult;
                break;
            default:
                result = cResultJoinFailed;
                break;
            }
            if (m_callerContext->m_state < common::CallContext::cStateSuccess ||
                m_callerContext->m_state > common::CallContext::cStateCancelled)
                m_callerContext->SignalFailure(result);
            common::g_sessionBeginMonitoringContent.m_joinResult = result;
            common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
            g_piaMesh->ProcessingAtMeshBegin();
            m_callerContext = 0;
        }
        CleanupInternalContexts();
        setStep(&JoinMeshJob::StartDisconnectStations, "JoinMeshJob::StartDisconnectStations");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    unsigned short connectedCount = 0;
    bool waiting = false;
    for (unsigned short index = 0; index < m_stationCount; ++index)
    {
        if (m_stationIndices[index] == static_cast<unsigned int>(localStationIndex))
        {
            ++connectedCount;
        }
        else if (m_stationConnected[index] == 0)
        {
            if (transport::g_stationConnectionInfoTable->GetStation(m_stationInfos[index]) != 0 &&
                g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(m_stationIndices[index])))
            {
                ++connectedCount;
            }
            else
            {
                waiting = true;
                break;
            }
        }
    }
    if (waiting || (m_stationCount != 0 && connectedCount == cLocalAndHostStationCount))
    {
        if (CheckTransportConnectionStatus() != 0)
            return MakeExecuteResult(cExecuteKindFinished);
        if (CheckConnectionStateWithHostStation() != 0)
        {
            setStep(&JoinMeshJob::StartDisconnectStations, "JoinMeshJob::StartDisconnectStations");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (static_cast<long long>(g_piaMesh->GetTime().m_tick) < 0)
        return MakeExecuteResult(cExecuteKindNextStep);
    g_piaMesh->SetMeshState(Mesh::cStateCreated);
    g_piaMesh->m_disconnectReason = 0;
    return SetNextStepAfterWaitAllConnection();
}

JoinMeshJob::ExecuteResult JoinMeshJob::CompleteProcess()
{
    if (m_callerContext != 0)
    {
        m_callerContext->SignalSuccess(0);
        m_callerContext = 0;
    }
    m_joinState = 4;
    g_piaMesh->SetMeshStartTime(common::g_piaJobScheduler->m_dispatchTime);
    CalcProcessingTimeOfJoin();
    CheckRelayConnectionForMonitoring();
    common::g_sessionBeginMonitoringContent.m_joinResult = 0;
    common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = 0xffffffff;
    common::g_sessionBeginMonitoringContent.m_joinMeshState = cMonitoringByteUnset;
    transport::g_piaTransport->SetMonitoringNetworkRtt(true);
    g_piaMesh->ProcessingAtMeshBegin();
    return MakeExecuteResult(cExecuteKindFinished);
}

JoinMeshJob::ExecuteResult JoinMeshJob::SetupLocalPlayerInfoFailure()
{
    if (m_callerContext->m_state < common::CallContext::cStateSuccess ||
        m_callerContext->m_state > common::CallContext::cStateCancelled)
        m_callerContext->SignalFailure(cResultSetupLocalPlayerInfoFailed);
    common::g_sessionBeginMonitoringContent.m_joinResult = cResultSetupLocalPlayerInfoFailed;
    common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
    g_piaMesh->ProcessingAtMeshBegin();
    return MakeExecuteResult(cExecuteKindFinished);
}

JoinMeshJob::ExecuteResult JoinMeshJob::StartDisconnectStations()
{
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation == 0)
        return MakeExecuteResult(cExecuteKindFinished);
    transport::Station** station = transport::g_piaStationManager->m_stations.data;
    if (station != station + transport::g_piaStationManager->m_stations.size)
    {
        do
        {
            if (*station != localStation)
            {
                if (g_piaMesh->IsValidStationIndex(static_cast<StationIndex>((*station)->m_field10)))
                    g_piaMesh->UnfixDisconnectedId(static_cast<StationIndex>((*station)->m_field10));
                (*station)->m_connectStationJob->Cleanup();
                (*station)->m_processConnectionRequestJob->Cleanup();
                transport::DisconnectStationJob* disconnectJob = (*station)->m_disconnectStationJob;
                if (disconnectJob == 0)
                {
                    (*station)->Trace(cTraceStationLevel);
                }
                else
                {
                    disconnectJob->Startup(*station);
                    disconnectJob->Ready(false);
                }
            }
            ++station;
        } while (station != transport::g_piaStationManager->m_stations.data + transport::g_piaStationManager->m_stations.size);
    }
    m_disconnectDeadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick +
        common::TimeSpan::GetTicksPerMSec().m_tick * cDisconnectTimeoutMsec;
    setStep(&JoinMeshJob::WaitDisconnectStations, "JoinMeshJob::WaitDisconnectStations");
    return MakeExecuteResult(cExecuteKindContinue);
}

JoinMeshJob::ExecuteResult JoinMeshJob::WaitDisconnectStations()
{
    transport::Station** station = transport::g_piaStationManager->m_stations.data;
    if (station != station + transport::g_piaStationManager->m_stations.size)
    {
        do
        {
            if ((*station)->m_field20 == cConnectedStationState && *station != transport::g_piaStationManager->m_localStation)
            {
                if (common::g_piaJobScheduler->m_dispatchTime < m_disconnectDeadline)
                    return MakeExecuteResult(cExecuteKindNextStep);
                break;
            }
            ++station;
        } while (station != transport::g_piaStationManager->m_stations.data + transport::g_piaStationManager->m_stations.size);
    }
    return MakeExecuteResult(cExecuteKindFinished);
}

JoinMeshJob::ExecuteResult JoinMeshJob::LeaveMeshWithHostMigration()
{
    if (g_piaMesh->GetStationNum() > 1)
    {
        g_piaMesh->LeaveMeshWithHostMigration(&m_callContext);
        setStep(&JoinMeshJob::WaitLeaveMesh, "JoinMeshJob::WaitLeaveMesh");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    setStep(&JoinMeshJob::CompleteCancelWithLeaveMesh, "JoinMeshJob::CompleteCancelWithLeaveMesh");
    return MakeExecuteResult(cExecuteKindContinue);
}

JoinMeshJob::ExecuteResult JoinMeshJob::LeaveMesh()
{
    g_piaMesh->LeaveMesh(&m_callContext);
    setStep(&JoinMeshJob::WaitLeaveMesh, "JoinMeshJob::WaitLeaveMesh");
    return MakeExecuteResult(cExecuteKindNextStep);
}

JoinMeshJob::ExecuteResult JoinMeshJob::WaitLeaveMesh()
{
    if (m_callContext.m_state >= common::CallContext::cStateSuccess &&
        m_callContext.m_state <= common::CallContext::cStateCancelled)
    {
        setStep(&JoinMeshJob::CompleteCancelWithLeaveMesh, "JoinMeshJob::CompleteCancelWithLeaveMesh");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

JoinMeshJob::ExecuteResult JoinMeshJob::CompleteCancelWithLeaveMesh()
{
    g_piaMesh->m_disconnectReason = 2;
    CleanupInternalContexts();
    m_callerContext->SignalCancel();
    m_callerContext = 0;
    CalcProcessingTimeOfJoin();
    CheckRelayConnectionForMonitoring();
    common::g_sessionBeginMonitoringContent.m_joinResult = cResultCancelled;
    common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
    common::g_sessionBeginMonitoringContent.m_joinMeshState = static_cast<unsigned char>(m_joinState);
    g_piaMesh->ProcessingAtMeshBegin();
    return MakeExecuteResult(cExecuteKindFinished);
}

void JoinMeshJob::Cleanup(Result result)
{
    if (IsRunning() && IsBackground())
    {
        m_cancelRequested = true;
        do
        {
            WaitForCompletion(2);
        } while (IsRunning());
    }
    CleanupImpl();
    CleanupInternalContexts();
    CleanupStations();
    m_waitingJoinResponse = false;
    for (int index = 0; index < cReceivedFlagNum; ++index)
        m_received[index] = false;
    if (m_requestAckId != 0)
    {
        if (transport::g_piaResendingMessageManager != 0)
            transport::g_piaResendingMessageManager->StopResending(m_requestAckId);
        m_requestAckId = 0;
    }
    m_lastUpdateCount = 0;
    for (unsigned int index = 0; index < m_stationCapacity; ++index)
        m_stationConnected[index] = 0;
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.Cancel();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
        {
            CalcProcessingTimeOfJoin();
            CheckRelayConnectionForMonitoring();
            if (result == 0)
            {
                if (m_callerContext->m_state != common::CallContext::cStateSuccess &&
                    m_callerContext->m_state != common::CallContext::cStateFailure &&
                    m_callerContext->m_state != common::CallContext::cStateCancelled)
                    m_callerContext->SignalFailure(cResultCancelled);
                common::g_sessionBeginMonitoringContent.m_joinResult = cResultCancelled;
            }
            else
            {
                if (m_callerContext->m_state != common::CallContext::cStateSuccess &&
                    m_callerContext->m_state != common::CallContext::cStateFailure &&
                    m_callerContext->m_state != common::CallContext::cStateCancelled)
                    m_callerContext->SignalFailure(result);
                common::g_sessionBeginMonitoringContent.m_joinResult = result;
            }
            common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
            g_piaMesh->ProcessingAtMeshBegin();
        }
        m_callerContext = 0;
    }
    m_joinState = 0;
}

unsigned int JoinMeshJob::GetJoinMeshState() const
{
    return m_joinState;
}

void JoinMeshJob::Trace(unsigned long long) const
{
}

unsigned int JoinMeshJob::StartupImpl()
{
    Reset(true);
    setStep(&JoinMeshJob::SetupLocalPlayerInfo, "JoinMeshJob::SetupLocalPlayerInfo");
    return 1;
}

void JoinMeshJob::CleanupImpl()
{
}

void JoinMeshJob::SetHostInfoToMonitoringData(const transport::StationConnectionInfo&)
{
}

void JoinMeshJob::SetNextStepAtSetupLocalPlayerInfoImpl()
{
    setStep(&JoinMeshJob::SetupLocalStation, "JoinMeshJob::SetupLocalStation");
}

JoinMeshJob::ExecuteResult JoinMeshJob::SetNextStepAfterWaitAllConnection()
{
    setStep(&JoinMeshJob::CompleteProcess, "JoinMeshJob::CompleteProcess");
    return MakeExecuteResult(cExecuteKindContinue);
}

void JoinMeshJob::CleanupInternalContexts()
{
    m_internalContext->SignalCancel();
    m_internalContext->Reset();
}

}}}
