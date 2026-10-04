#include "nn/pia/session/Mesh.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/session/DestroyMeshJob.hpp"
#include "nn/pia/session/JoinMeshJob.hpp"
#include "nn/pia/session/KickoutManageJob.hpp"
#include "nn/pia/session/LeaveMeshJob.hpp"
#include "nn/pia/session/LeaveWithHostMigrationJob.hpp"
#include "nn/pia/session/MeshEventListener.hpp"
#include "nn/pia/session/CreateMeshJob.hpp"
#include "nn/pia/session/SyncClockProtocol.hpp"
#include "nn/pia/transport/BandwidthCheckProtocol.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/transport/ThreadStreamManager.hpp"
#include "nn/pia/transport/TransportAnalyzer.hpp"

#include <new>

namespace nn { namespace pia { namespace session {

Mesh* g_piaMesh = 0;

namespace
{

unsigned int sDebugSetting = 0;

}

Mesh::Mesh(const Setting&)
    : m_createMeshJob(0),
      m_joinMeshJob(0),
      m_leaveMeshJob(0),
      m_unknown0c(0),
      m_processUpdateMeshJob(0),
      m_destroyMeshJob(0),
      m_processDestroyMeshJob(0),
      m_processHostMigrationJob(0),
      m_leaveWithHostMigrationJob(0),
      m_unknown24(0),
      m_kickoutManageJob(0),
      m_unknown2c(0),
      m_meshEventCallback(0),
      m_joiningMeshApprovalCallback(0),
      m_hostCandidateRankingCallback(0),
      m_getSessionIdCallback(0),
      m_meshProtocolId(0),
      m_meshProtocol(0),
      m_stationProtocol(0),
      m_bandwidthCheckProtocolId(0),
      m_bandwidthCheckProtocol(0),
      m_unknown54(0),
      m_unknown56(1),
      m_unknown58(static_cast<StationIndex>(0xfd)),
      m_unknown5c(static_cast<StationIndex>(0xfd)),
      m_meshState(0),
      m_disconnectReason(0),
      m_hostMigrationMode(0),
      m_bitRateCheckMode(0),
      m_hostMigrationEnabled(1),
      m_unknown75(0),
      m_recentPlayRecordEnabled(0),
      m_unknown77(0),
      m_connectedStationBitmap(0),
      m_validStationBitmap(0),
      m_unknown80(0xffffffffu),
      m_unknown88(0),
      m_unknown8c(0),
      m_syncClockProtocolId(0),
      m_meshStartTime(0),
      m_meshStartTimeValid(0),
      m_unknowna1(0),
      m_hostMigrationStarted(0),
      m_unknowna3(1),
      m_monitoringDataSender(0),
      m_asyncProcess(0),
      m_meshEventListener(0),
      m_skipReport(0),
      m_skipCleanupReport(0),
      m_unknownc6(1),
      m_unknownc7(1),
      m_unknownc8(0xffffffffu),
      m_unknowncc(0),
      m_denyJoinRequestByInvalidState(0),
      m_denyJoinRequestBySessionIsFull(0)
{
}

Mesh::~Mesh()
{
    Cleanup();
}

Result Mesh::Initialize(const Setting& setting, bool)
{
    m_networkTopologyType = setting.m_field04;
    m_bitRateCheckMode = setting.m_field08;
    return 0;
}

Result Mesh::CreateInstance(const Setting& setting)
{
    if (g_piaMesh != 0)
        return static_cast<Result>(0x10c0f);
    g_piaMesh = new (std::nothrow) Mesh(setting);
    if (g_piaMesh == 0)
        return static_cast<Result>(0x10c01);
    return g_piaMesh->Initialize(setting, false);
}

void Mesh::DestroyInstance()
{
    delete g_piaMesh;
    g_piaMesh = 0;
}

void Mesh::CleanupStationsJobs()
{
    m_connectedStationBitmap = 0;
    m_validStationBitmap = 0;
}

void Mesh::CleanupJobs()
{
    m_kickoutManageJob = 0;
    m_meshProtocol = 0;
    m_bandwidthCheckProtocol = 0;
}

bool Mesh::IsValidStationIndex(StationIndex index) const
{
    if (static_cast<int>(index) > 31)
        return false;
    unsigned int mask = 1;
    unsigned int shift = index & 0xffff;
    while (shift != 0)
    {
        mask <<= 1;
        --shift;
    }
    return (m_validStationBitmap & mask) != 0;
}

void Mesh::StopUse(StationIndex index)
{
    if (static_cast<int>(index) > 31)
        return;
    unsigned int mask = 1;
    unsigned int shift = index & 0xffff;
    while (shift != 0)
    {
        mask <<= 1;
        --shift;
    }
    m_validStationBitmap &= ~mask;
    m_connectedStationBitmap &= ~mask;
}

void Mesh::UnfixDisconnectedId(StationIndex index)
{
    StopUse(index);
}

void Mesh::ResetStationIndexUsage()
{
    m_connectedStationBitmap = 0;
    m_validStationBitmap = 0;
}

void Mesh::SetMeshState(State state)
{
    m_meshState = static_cast<unsigned int>(state);
}

void Mesh::CleanupStatus()
{
    m_disconnectReason = 0;
    SetMeshState(cStateNone);
    ResetStationIndexUsage();
}

void Mesh::Cleanup()
{
    CleanupJobs();
    CleanupStationsJobs();
    CleanupStatus();
    m_callContext.Reset();
    m_asyncProcess = 0;
}

Result Mesh::CheckPlayerInfo(const transport::Station::PlayerInfo& info) const
{
    if (info.m_name.GetStringLength() == 0 && info.m_name.GetEncodingType() == 0 &&
        info.m_language == 0xff)
        return static_cast<Result>(0x10c07);
    if (info.m_name.GetStringLength() >= 21)
        return static_cast<Result>(0x10c07);
    if (info.m_name.GetStringLength() != 0)
    {
        if (info.m_name.GetEncodingType() != common::String::EncodingType_Utf16)
            return static_cast<Result>(0x10c07);
        if (info.m_language > 11)
            return static_cast<Result>(0x10c07);
    }
    return 0;
}

Result Mesh::CheckPlayerInfoSetting(const transport::Station::PlayerInfo* infos,
                                    unsigned char count) const
{
    if (infos == 0)
        return static_cast<Result>(0x10c07);
    if (count == 0)
        return static_cast<Result>(0x10c07);
    if (count < 2)
    {
        for (int i = 0; i < count; ++i)
        {
            if (CheckPlayerInfo(infos[i]) != 0)
                return static_cast<Result>(0x10c07);
        }
        return 0;
    }
    return static_cast<Result>(0x10c07);
}

Result Mesh::Startup(StartupSetting& setting)
{
    const Result result = CheckPlayerInfoSetting(setting.m_playerInfo,
                                                  setting.m_playerCount);
    if (result != 0)
        return result;
    SetMeshState(cStateStarted);
    return 0;
}

Result Mesh::CreateMesh(common::CallContext* callerContext)
{
    if (callerContext != 0 && callerContext->m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    if (m_meshState == cStateCreated)
        return static_cast<Result>(0x10c08);
    if (m_createMeshJob->GetState() != common::Job::cStateNotEntried &&
        m_createMeshJob->GetState() != common::Job::cStateCompleted)
        return static_cast<Result>(0x10c08);
    m_createMeshJob->Startup(callerContext);
    m_createMeshJob->Ready(true);
    return 0;
}

Result Mesh::CreateMeshAsync()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    ResetInternalAsyncState();
    const Result result = CreateMesh(&m_callContext);
    if (result == 0)
        m_asyncProcess = cAsyncCreateMesh;
    return result;
}

bool Mesh::IsAsyncProcessCompleted(AsyncProcessingName process) const
{
    if (m_asyncProcess == static_cast<unsigned int>(process))
        return m_callContext.m_state - 2u < 3u;
    return false;
}

bool Mesh::IsCreateMeshCompleted() const
{
    return IsAsyncProcessCompleted(cAsyncCreateMesh);
}

Result Mesh::GetAsyncProcessResult(AsyncProcessingName process) const
{
    if (m_asyncProcess == static_cast<unsigned int>(process) &&
        m_callContext.m_state - 2u < 3u)
        return m_callContext.m_result;
    return static_cast<Result>(0x10c08);
}

Result Mesh::GetCreateMeshResult() const
{
    return GetAsyncProcessResult(cAsyncCreateMesh);
}

Result Mesh::CancelAsyncProcess(AsyncProcessingName process)
{
    if (m_asyncProcess != static_cast<unsigned int>(process) ||
        m_callContext.m_state != common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    m_callContext.Cancel();
    return 0;
}

unsigned int Mesh::GetHostMigrationMode() const
{
    return m_hostMigrationMode;
}

bool Mesh::IsRecentPlayRecordEnabled() const
{
    return m_recentPlayRecordEnabled != 0;
}

KickoutManageJob* Mesh::GetKickoutManageJobPtr()
{
    return m_kickoutManageJob;
}

void Mesh::SetSyncClockPulseInterval(int interval)
{
    if (transport::g_piaTransport == 0)
        return;

    transport::Protocol* protocol =
        transport::g_piaTransport->m_protocolManager.SearchProtocol(
            m_syncClockProtocolId, SyncClockProtocol::cProtocolType);
    if (protocol != 0)
        static_cast<SyncClockProtocol*>(protocol)->m_pulseInterval = interval;
}

void Mesh::SetSkipCleanupReport(bool skip)
{
    m_skipCleanupReport = skip;
}

Result Mesh::JoinMeshAsync(const transport::StationConnectionInfo& info)
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    ResetInternalAsyncState();
    const Result result = JoinMeshCore(info, &m_callContext);
    if (result == 0)
        m_asyncProcess = cAsyncJoinMesh;
    return result;
}

void Mesh::ResetInternalAsyncState()
{
    if (m_asyncProcess != 0 && m_callContext.m_state != common::CallContext::cStateInProgress)
    {
        m_callContext.Reset();
        m_asyncProcess = 0;
    }
}

bool Mesh::IsJoinMeshCompleted() const
{
    return IsAsyncProcessCompleted(cAsyncJoinMesh);
}

Result Mesh::GetJoinMeshResult() const
{
    return GetAsyncProcessResult(cAsyncJoinMesh);
}

Result Mesh::CancelJoinMeshAsync()
{
    return CancelAsyncProcess(cAsyncJoinMesh);
}

Result Mesh::LeaveMeshAsync()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    ResetInternalAsyncState();
    const Result result = LeaveMesh(&m_callContext);
    if (result == 0)
        m_asyncProcess = cAsyncLeaveMesh;
    return result;
}

bool Mesh::IsLeaveMeshCompleted() const
{
    return IsAsyncProcessCompleted(cAsyncLeaveMesh);
}

Result Mesh::GetLeaveMeshResult() const
{
    return GetAsyncProcessResult(cAsyncLeaveMesh);
}

Result Mesh::LeaveMeshWithHostMigrationAsync()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c07);
    ResetInternalAsyncState();
    const Result result = LeaveMeshWithHostMigration(&m_callContext);
    if (result == 0)
        m_asyncProcess = cAsyncLeaveMeshWithHostMigration;
    return result;
}

bool Mesh::IsLeaveMeshWithHostMigrationCompleted() const
{
    return IsAsyncProcessCompleted(cAsyncLeaveMeshWithHostMigration);
}

Result Mesh::GetLeaveMeshWithHostMigrationResult() const
{
    return GetAsyncProcessResult(cAsyncLeaveMeshWithHostMigration);
}

Result Mesh::DestroyMeshAsync()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c07);
    ResetInternalAsyncState();
    const Result result = DestroyMesh(&m_callContext);
    if (result == 0)
        m_asyncProcess = cAsyncDestroyMesh;
    return result;
}

bool Mesh::IsDestroyMeshCompleted() const
{
    return IsAsyncProcessCompleted(cAsyncDestroyMesh);
}

Result Mesh::GetDestroyMeshResult() const
{
    return GetAsyncProcessResult(cAsyncDestroyMesh);
}

unsigned int Mesh::GetJoinMeshState() const
{
    if (m_joinMeshJob == 0)
        return 0;
    return m_joinMeshJob->GetJoinMeshState();
}

StationIndex Mesh::GetHostStationIndex() const
{
    return m_unknown58;
}

StationIndex Mesh::GetLocalStationIndex() const
{
    return m_unknown5c;
}

transport::BandwidthCheckProtocol* Mesh::GetBandwidthCheckProtocol()
{
    return m_bandwidthCheckProtocol;
}

void Mesh::RegisterMeshEventCallback(MeshEventCallback callback)
{
    m_meshEventCallback = callback;
}

void Mesh::UnregisterMeshEventCallback()
{
    m_meshEventCallback = 0;
}

void Mesh::RegisterJoiningMeshApprovalCallback(JoiningMeshApprovalCallback callback)
{
    m_joiningMeshApprovalCallback = callback;
}

void Mesh::UnregisterJoiningMeshApprovalCallback()
{
    m_joiningMeshApprovalCallback = 0;
}

Result Mesh::RegisterCheckHostCandidateRankingCallback(HostCandidateRankingCallback callback)
{
    if (m_hostMigrationMode != 2)
        return static_cast<Result>(0x10c08);
    m_hostCandidateRankingCallback = callback;
    return 0;
}

void Mesh::UnregisterCheckHostCandidateRankingCallback()
{
    m_hostCandidateRankingCallback = 0;
}

void Mesh::RegisterEventListener(MeshEventListener* listener)
{
    m_meshEventListener = listener;
}

void Mesh::UnregisterEventListener()
{
    m_meshEventListener = 0;
}

void Mesh::RegisterGetSessionIdCallback(GetSessionIdCallback callback)
{
    m_getSessionIdCallback = callback;
}

void Mesh::UnregisterGetSessionIdCallback()
{
    m_getSessionIdCallback = 0;
}

MeshProtocol* Mesh::GetMeshProtocol()
{
    return m_meshProtocol;
}

void* Mesh::GetStationProtocol()
{
    return m_stationProtocol;
}

void Mesh::SetHostMigrationStarted(bool started)
{
    m_hostMigrationStarted = started;
}

bool Mesh::IsHostMigrationStarted()
{
    return m_hostMigrationStarted;
}

void Mesh::SetFatalError(DisconnectReason reason)
{
    g_piaMesh->m_disconnectReason = static_cast<unsigned int>(reason);
    if (static_cast<unsigned int>(reason) == 9)
    {
        g_piaMesh->SetMeshState(cStateNone);
        return;
    }
    g_piaMesh->ProcessingAtMeshEnd(reason);
    g_piaMesh->CleanupStatus();
}

unsigned int Mesh::GetNetworkTopologyType()
{
    return m_networkTopologyType;
}

bool Mesh::IsProcessingLeaveMesh() const
{
    bool running = false;
    if (m_leaveMeshJob != 0)
        running = m_leaveMeshJob->IsRunning();
    if (m_leaveWithHostMigrationJob != 0)
        running |= m_leaveWithHostMigrationJob->IsRunning();
    if (m_destroyMeshJob != 0)
        running |= m_destroyMeshJob->IsRunning();
    return running;
}

unsigned int Mesh::GetLocalPlayerCount() const
{
    return m_unknownc7;
}

bool Mesh::IsPlayersCountedAsParticipants() const
{
    return m_unknownc6;
}

common::MonitoringDataSender* Mesh::GetMonitoringDataSender()
{
    return m_monitoringDataSender;
}

unsigned int Mesh::GetBitRateCheckMode()
{
    return m_bitRateCheckMode;
}

void Mesh::SetDenyJoinRequestByInvalidState(bool deny)
{
    m_denyJoinRequestByInvalidState = deny;
}

void Mesh::SetDenyJoinRequestBySessionIsFull(bool deny)
{
    m_denyJoinRequestBySessionIsFull = deny;
}

void Mesh::StartUse(StationIndex index)
{
    if (static_cast<int>(index) > 31)
        return;
    unsigned int mask = 1;
    unsigned int shift = index & 0xffff;
    while (shift != 0)
    {
        mask <<= 1;
        --shift;
    }
    m_validStationBitmap |= mask;
}

void Mesh::FixConnectedId(StationIndex index)
{
    StartUse(index);
    NoticeMeshEvent(cEventTypeJoin, index);
}

void Mesh::JoinResponseComplete(StationIndex index)
{
    if (static_cast<int>(index) > 31)
        return;
    unsigned int mask = 1;
    unsigned int shift = index & 0xffff;
    while (shift != 0)
    {
        mask <<= 1;
        --shift;
    }
    m_connectedStationBitmap |= mask;
}

void Mesh::SetMeshStartTime(common::Time time)
{
    m_meshStartTimeValid = 1;
    m_meshStartTime = time.m_tick;
}

void Mesh::ProcessingAtEndHM()
{
}

void Mesh::AddPlayHistory(transport::Station*, transport::Station::PlayerInfo*, unsigned int count)
{
    (void)count;
}

void Mesh::NoticeMeshEvent(EventType type, StationIndex index)
{
    if (type == cEventTypeJoin)
    {
        transport::Station* station = transport::g_piaStationManager->GetStation(index);
        if (station != 0 && m_unknowna3 != 0 && m_recentPlayRecordEnabled != 0)
            AddPlayHistory(station, m_playerInfo, 1);
    }
    if (m_meshEventCallback != 0)
    {
        unsigned long long stationId;
        transport::g_piaTransport->ConvertToStationId(&stationId, index);
        m_meshEventCallback(type, stationId);
    }
    if (m_meshEventListener != 0)
    {
        MeshEventListener::MeshEvent event;
        event.type = type;
        event.stationIndex = index;
        event.stationId = 0;
        m_meshEventListener->NoticeEvent(event);
    }
}

unsigned short Mesh::GetStationNum() const
{
    const unsigned int bitmap = IsHost() ? m_connectedStationBitmap : m_validStationBitmap;
    const unsigned int local = m_unknown5c & 0xffff;
    unsigned short count = 1;
    for (unsigned int i = 0; i < 32; ++i)
    {
        if (i != local && (bitmap & (1u << i)) != 0)
            ++count;
    }
    return count;
}

bool Mesh::IsHost() const
{
    bool isHost = false;
    if (static_cast<int>(m_unknown5c) <= 31)
        isHost = m_unknown5c == m_unknown58;
    return isHost;
}

bool Mesh::IsMeshStartTimeValid() const
{
    return m_meshStartTimeValid != 0;
}

common::TimeSpan Mesh::CalcDiffWithMeshStartTime(common::Time time) const
{
    common::TimeSpan span;
    span.m_tick = static_cast<long long>(time.m_tick - m_meshStartTime);
    return span;
}

void Mesh::ProcessingAtMeshEnd(DisconnectReason reason)
{
    MonitoringProcess(reason, 1);
}

Result Mesh::DestroyMesh(common::CallContext* callerContext)
{
    if (m_unknown5c == static_cast<StationIndex>(0xfd) || m_unknown5c != m_unknown58)
        return static_cast<Result>(0x10c08);
    if (callerContext != 0 && callerContext->m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    m_unknown56 = 0;
    if (GetStationNum() < 2)
    {
        m_disconnectReason = 2;
        ProcessingAtMeshEnd(static_cast<DisconnectReason>(m_disconnectReason));
        Cleanup();
        if (callerContext != 0)
        {
            callerContext->InitiateCall();
            callerContext->SignalSuccess(0);
        }
    }
    else if (m_destroyMeshJob->Startup(callerContext, false) == 0)
        m_destroyMeshJob->AssociateSystemWith(callerContext);
    else
        m_destroyMeshJob->Ready(false);
    return 0;
}

bool Mesh::IsHostMigrationEnabled() const
{
    return m_hostMigrationEnabled;
}

Result Mesh::LeaveMeshWithHostMigration(common::CallContext* callerContext)
{
    if (!IsHostMigrationEnabled())
        return static_cast<Result>(0x10c08);
    if (m_unknown5c == static_cast<StationIndex>(0xfd) || m_unknown5c != m_unknown58)
        return static_cast<Result>(0x10c08);
    if (callerContext != 0 && callerContext->m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    m_unknown56 = 0;
    if (GetStationNum() < 2)
    {
        m_disconnectReason = 2;
        ProcessingAtMeshEnd(static_cast<DisconnectReason>(m_disconnectReason));
        Cleanup();
        if (callerContext != 0)
        {
            callerContext->InitiateCall();
            callerContext->SignalSuccess(0);
        }
    }
    else if (m_leaveWithHostMigrationJob->Startup(callerContext) != 0)
        m_leaveWithHostMigrationJob->Ready(false);
    return 0;
}

Result Mesh::LeaveMesh(common::CallContext* callerContext)
{
    if (callerContext != 0 && callerContext->m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    if (m_meshState == cStateNone)
    {
        if (callerContext != 0)
        {
            callerContext->InitiateCall();
            callerContext->SignalSuccess(0);
        }
        return 0;
    }
    if (m_unknown5c == static_cast<StationIndex>(0xfd) || m_unknown5c == m_unknown58)
        return static_cast<Result>(0x10c08);
    if (m_leaveMeshJob->Startup(callerContext) == 0)
    {
        if ((!m_kickoutManageJob->IsRunning() || !m_kickoutManageJob->AssociateKickoutWith(callerContext)) &&
            m_leaveMeshJob->RegisterExtraCallback(callerContext) == 0)
            return static_cast<Result>(0x10c08);
    }
    else
        m_leaveMeshJob->Ready(false);
    return 0;
}

StationIndex Mesh::GetUsableStationIndex()
{
    for (unsigned int index = 0; index < 32; ++index)
    {
        if ((m_validStationBitmap & (1u << index)) == 0)
            return static_cast<StationIndex>(index);
    }
    return static_cast<StationIndex>(0xfd);
}

Result Mesh::CheckApprovalJoin(transport::Station*)
{
    if (m_unknown56 == 0 || IsProcessingLeaveMesh())
        return static_cast<Result>(2);
    if (m_unknown54 <= GetStationNum() || GetUsableStationIndex() == static_cast<StationIndex>(0xfd))
        return 0;
    if (m_denyJoinRequestBySessionIsFull != 0)
    {
        m_denyJoinRequestBySessionIsFull = 0;
        return 0;
    }
    if (m_denyJoinRequestByInvalidState != 0)
    {
        m_denyJoinRequestByInvalidState = 0;
        return static_cast<Result>(4);
    }
    return static_cast<Result>(0xff);
}

Result Mesh::JoinMeshCore(const transport::StationConnectionInfo& info,
                          common::CallContext* callerContext)
{
    if (m_meshState == cStateCreated)
        return static_cast<Result>(0x10c08);
    if (callerContext != 0 && callerContext->m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    m_unknown88 = 0;
    m_unknown58 = static_cast<StationIndex>(0xfe);
    m_unknown80 = 0;
    m_unknowncc = 0;
    m_unknown8c = 0;
    m_unknownc8 = 0;
    if (m_joinMeshJob->GetState() != common::Job::cStateNotEntried &&
        m_joinMeshJob->GetState() != common::Job::cStateCompleted)
        m_joinMeshJob->Reset(true);
    if (m_joinMeshJob->Startup(info, callerContext))
        m_joinMeshJob->Ready(true);
    return 0;
}

ProcessHostMigrationJob* Mesh::GetProcessHostMigrationJobPtr()
{
    return m_processHostMigrationJob;
}

ProcessDestroyMeshJob* Mesh::GetProcessDestroyMeshJobPtr()
{
    return m_processDestroyMeshJob;
}

ProcessUpdateMeshJob* Mesh::GetProcessUpdateMeshJobPtr()
{
    return m_processUpdateMeshJob;
}

Result Mesh::setDebugSettingImpl(const DebugSetting& setting)
{
    if (g_piaMesh != 0)
        return static_cast<Result>(0x10c08);
    sDebugSetting = setting.m_field00 & 0xffff0000u;
    return 0;
}

void Mesh::SetDebugSettingForInternal(const DebugSetting& setting)
{
    setDebugSettingImpl(setting);
}

void Mesh::TraceStationConnectionInfoList(unsigned long long)
{
}

void Mesh::SetSkipReport(bool skip)
{
    m_skipReport = skip;
}

void Mesh::SendReport(unsigned char reason)
{
    if (m_monitoringDataSender != 0)
        m_monitoringDataSender->RequestSendReport(
            static_cast<common::MonitoringDataSender::DataType>(reason));
}

unsigned int Mesh::GetMeshState() const
{
    return m_meshState;
}

Result Mesh::CheckConnectionError() const
{
    if (m_meshState == cStateCreated)
        return 0;
    return static_cast<Result>(0x6c50);
}

unsigned int Mesh::GetDisconnectReason() const
{
    if (m_meshState == cStateCreated)
        return 0;
    if (m_disconnectReason == 1)
        return 1;
    if (m_kickoutManageJob->m_kickoutReason == 0)
        return m_disconnectReason;
    if (m_kickoutManageJob->m_kickoutReason != 2)
        return 5;
    return 4;
}

bool Mesh::IsBandwidthCheckProtocolSetupParamValid(int measurementTime,
                                                    unsigned int ipPacketSize,
                                                    int measurementCount) const
{
    if (m_bitRateCheckMode == 0)
        return true;
    return m_bandwidthCheckProtocol->IsSetupParamValid(measurementTime, ipPacketSize, measurementCount);
}

}}}
