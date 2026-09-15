#include "nn/pia/session/Mesh.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/session/KickoutManageJob.hpp"
#include "nn/pia/session/SyncClockProtocol.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/transport/ThreadStreamManager.hpp"
#include "nn/pia/transport/TransportAnalyzer.hpp"

#include <new>

namespace nn { namespace pia { namespace session {

Mesh* g_piaMesh = 0;

Mesh::Mesh(const Setting& setting)
    : m_kickoutManageJob(0),
      m_syncClockProtocolId(0),
      m_meshProtocol(0),
      m_bandwidthCheckProtocol(0),
      m_meshState(0),
      m_disconnectReason(0),
      m_recentPlayRecordEnabled(0),
      m_connectedStationBitmap(0),
      m_validStationBitmap(0),
      m_monitoringDataSender(0),
      m_asyncProcess(0),
      m_meshEventListener(0),
      m_skipReport(0),
      m_skipCleanupReport(0)
{
    m_createMeshJob = 0;
    m_joinMeshJob = 0;
    m_leaveMeshJob = 0;
    m_unknown0c = 0;
    m_processUpdateMeshJob = 0;
    m_unknown14 = 0;
    m_unknown18 = 0;
    m_processHostMigrationJob = 0;
    m_unknown20 = 0;
    m_unknown24 = 0;
    m_joiningMeshApprovalCallback = 0;
    m_hostCandidateRankingCallback = 0;
    m_getSessionIdCallback = 0;
    m_unknown3c = 0;
    m_unknown48 = 0;
    m_unknown58 = 0;
    m_unknown5c = 0xfd;
    m_unknown60 = 0;
    m_unknown6c = 0;
    m_unknown70 = 0;
    m_unknown74 = 1;
    m_unknown75 = 0;
    m_unknown76 = 0;
    m_unknown77 = 0;
    m_unknown84[0] = m_unknown84[1] = m_unknown84[2] = m_unknown84[3] = 0;
    m_unknown88 = 0; m_unknown8c = 0; m_unknown90 = 0;
    for (unsigned int i = 0; i < sizeof(m_unknown92); ++i) m_unknown92[i] = 0;
    m_unknown98 = 0; m_unknown9c = 0;
    m_meshStartTime = 0;
    m_meshStartTimeValid = 0;
    m_localPlayerCount = 0;
    m_playersCountedAsParticipants = 0;
    m_recentPlayRecordEnabled = 0;
    m_unknownc8 = 0xffffffffu;
    m_unknowncc = 0;
    m_denyJoinRequestByInvalidState = 0;
    m_denyJoinRequestBySessionIsFull = 0;
    m_unknowncf = 0;
    m_playerInfoVector = 0;
    m_unknown138 = 0;
    for (unsigned int i = 0; i < sizeof(m_unknown139); ++i) m_unknown139[i] = 0;
    m_unknown140 = m_unknown144 = m_unknown148 = m_unknown14c = 0;
    for (unsigned int i = 0; i < sizeof(m_unknown151); ++i) m_unknown151[i] = 0;
    Initialize(setting, false);
}

Mesh::~Mesh()
{
    Cleanup();
}

Result Mesh::Initialize(const Setting& setting, bool isHost)
{
    m_meshState = cStateNone;
    m_disconnectReason = 0;
    m_unknown6c = setting.m_field00;
    m_recentPlayRecordEnabled = static_cast<unsigned char>(setting.m_field04 != 0);
    m_connectedStationBitmap = 0;
    m_validStationBitmap = 0;
    m_asyncProcess = 0;
    m_callContext.Reset();
    m_skipReport = 0;
    m_skipCleanupReport = 0;
    if (isHost)
        m_meshState = cStateCreated;
    return 0;
}

Result Mesh::CreateInstance(const Setting& setting)
{
    if (g_piaMesh != 0)
        return static_cast<Result>(0x10c08);
    g_piaMesh = new (std::nothrow) Mesh(setting);
    return g_piaMesh != 0 ? static_cast<Result>(0) :
                            static_cast<Result>(0x10c07);
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
    const unsigned int value = static_cast<unsigned int>(index);
    return value < 32 && (m_validStationBitmap & (1u << value)) != 0;
}

void Mesh::StopUse(StationIndex index)
{
    const unsigned int value = static_cast<unsigned int>(index);
    if (value < 32)
    {
        const unsigned int mask = 1u << value;
        m_validStationBitmap &= ~mask;
        m_connectedStationBitmap &= ~mask;
    }
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
    return info.m_name.GetStringLength() <= 20 ? static_cast<Result>(0) :
                                   static_cast<Result>(0x10c07);
}

Result Mesh::CheckPlayerInfoSetting(const transport::Station::PlayerInfo* infos,
                                    unsigned char count) const
{
    if (infos == 0 && count != 0)
        return static_cast<Result>(0x10c07);
    for (unsigned int i = 0; i < count; ++i)
    {
        const Result result = CheckPlayerInfo(infos[i]);
        if (result != 0)
            return result;
    }
    return 0;
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
    if (m_meshState == cStateStarted || m_unknown60 != 0)
        return static_cast<Result>(0x10c08);
    if (m_createMeshJob == 0)
        return static_cast<Result>(0x10c08);
    if (callerContext != 0 && !callerContext->InitiateCall())
        return static_cast<Result>(0x10c08);
    m_callContext.InitiateCall();
    /* The concrete job declaration is intentionally unresolved in this source
       tree; its startup is entered by the owning dispatcher after this call. */
    return 0;
}

Result Mesh::CreateMeshAsync()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    ResetInternalAsyncState();
    const Result r = CreateMesh(&m_callContext);
    if (r == 0) m_asyncProcess = cAsyncCreateMesh;
    return r;
}

bool Mesh::IsAsyncProcessCompleted(AsyncProcessingName process) const
{
    return m_asyncProcess == static_cast<unsigned int>(process) &&
           m_callContext.m_state != common::CallContext::cStateInProgress;
}

bool Mesh::IsCreateMeshCompleted() const
{
    return IsAsyncProcessCompleted(cAsyncCreateMesh);
}

Result Mesh::GetAsyncProcessResult(AsyncProcessingName process) const
{
    return IsAsyncProcessCompleted(process) ? m_callContext.m_result :
                                             static_cast<Result>(0x10c08);
}

Result Mesh::GetCreateMeshResult() const
{
    return GetAsyncProcessResult(cAsyncCreateMesh);
}

void Mesh::CancelAsyncProcess(AsyncProcessingName process)
{
    if (m_asyncProcess == static_cast<unsigned int>(process) &&
        m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.Cancel();
}

unsigned int Mesh::GetHostMigrationMode() const
{
    return m_unknown6c;
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
    {
        return static_cast<Result>(0x10c08);
    }
    ResetInternalAsyncState();
    const Result r = JoinMeshCore(info, &m_callContext);
    if (r == 0) m_asyncProcess = cAsyncJoinMesh;
    return r;
}

void Mesh::ResetInternalAsyncState()
{
    if (m_callContext.m_state != common::CallContext::cStateInProgress)
    {
        m_callContext.Reset();
        m_asyncProcess = 0;
    }
}

bool Mesh::IsJoinMeshCompleted() const
{
    return m_asyncProcess == cAsyncJoinMesh &&
           m_callContext.m_state != common::CallContext::cStateInProgress;
}

Result Mesh::GetJoinMeshResult() const
{
    return IsJoinMeshCompleted() ? m_callContext.m_result :
                                   static_cast<Result>(0x10c08);
}

void Mesh::CancelJoinMeshAsync()
{
    if (m_asyncProcess == cAsyncJoinMesh &&
        m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.Cancel();
}

Result Mesh::LeaveMeshAsync()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
    {
        return static_cast<Result>(0x10c08);
    }
    ResetInternalAsyncState();
    const Result r = LeaveMesh(&m_callContext);
    if (r == 0) m_asyncProcess = cAsyncLeaveMesh;
    return r;
}

bool Mesh::IsLeaveMeshCompleted() const
{
    return m_asyncProcess == cAsyncLeaveMesh &&
           m_callContext.m_state != common::CallContext::cStateInProgress;
}

Result Mesh::GetLeaveMeshResult() const
{
    return IsLeaveMeshCompleted() ? m_callContext.m_result :
                                    static_cast<Result>(0x10c08);
}

Result Mesh::LeaveMeshWithHostMigrationAsync()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
    {
        return static_cast<Result>(0x10c08);
    }
    ResetInternalAsyncState();
    const Result r = LeaveMeshWithHostMigration(&m_callContext);
    if (r == 0) m_asyncProcess = cAsyncLeaveMeshWithHostMigration;
    return r;
}

bool Mesh::IsLeaveMeshWithHostMigrationCompleted() const
{
    return m_asyncProcess == cAsyncLeaveMeshWithHostMigration &&
           m_callContext.m_state != common::CallContext::cStateInProgress;
}

Result Mesh::GetLeaveMeshWithHostMigrationResult() const
{
    return IsLeaveMeshWithHostMigrationCompleted() ? m_callContext.m_result :
        static_cast<Result>(0x10c08);
}

Result Mesh::DestroyMeshAsync()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
    {
        return static_cast<Result>(0x10c08);
    }
    ResetInternalAsyncState();
    const Result r = DestroyMesh(&m_callContext);
    if (r == 0) m_asyncProcess = cAsyncDestroyMesh;
    return r;
}

bool Mesh::IsDestroyMeshCompleted() const
{
    return m_asyncProcess == cAsyncDestroyMesh &&
           m_callContext.m_state != common::CallContext::cStateInProgress;
}

Result Mesh::GetDestroyMeshResult() const
{
    return IsDestroyMeshCompleted() ? m_callContext.m_result :
                                      static_cast<Result>(0x10c08);
}

unsigned int Mesh::GetJoinMeshState() const
{
    return m_meshState;
}

StationIndex Mesh::GetHostStationIndex() const
{
    return static_cast<StationIndex>((m_unknown5c >> 16) & 0xff);
}

StationIndex Mesh::GetLocalStationIndex() const
{
    return static_cast<StationIndex>(m_unknown5c & 0xff);
}

void* Mesh::GetBandwidthCheckProtocol()
{
    return m_bandwidthCheckProtocol;
}

void Mesh::RegisterJoiningMeshApprovalCallback(JoiningMeshApprovalCallback callback)
{
    m_joiningMeshApprovalCallback = callback;
}

void Mesh::UnregisterJoiningMeshApprovalCallback()
{
    m_joiningMeshApprovalCallback = 0;
}

void Mesh::RegisterCheckHostCandidateRankingCallback(HostCandidateRankingCallback callback)
{
    m_hostCandidateRankingCallback = callback;
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

void* Mesh::GetMeshProtocol()
{
    return m_meshProtocol;
}

void* Mesh::GetStationProtocol()
{
    return m_meshProtocol;
}

void Mesh::SetHostMigrationStarted(bool started)
{
    m_unknown6c = started ? 1u : 0u;
}

bool Mesh::IsHostMigrationStarted()
{
    return m_unknown6c != 0;
}

void Mesh::SetFatalError(DisconnectReason reason)
{
    m_disconnectReason = static_cast<unsigned int>(reason);
    if (static_cast<unsigned int>(reason) == 9)
    {
        SetMeshState(cStateNone);
        return;
    }
    ProcessingAtMeshEnd(reason);
    CleanupStatus();
}

unsigned int Mesh::GetNetworkTopologyType()
{
    return m_networkTopologyType;
}

void Mesh::SetupSendReport()
{
    m_skipReport = 0;
}

bool Mesh::IsProcessedAtMeshBegin() const
{
    return m_meshStartTimeValid != 0;
}

bool Mesh::IsProcessingHostMigration() const
{
    return m_unknown6c != 0 && m_meshState != cStateNone;
}

unsigned int Mesh::GetLocalPlayerCount() const
{
    return m_localPlayerCount;
}

bool Mesh::IsPlayersCountedAsParticipants() const
{
    return m_playersCountedAsParticipants != 0;
}

void Mesh::CleanupMonitoringBeginData()
{
    m_meshStartTimeValid = 0;
    m_localPlayerCount = 0;
    m_playersCountedAsParticipants = 0;
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
    m_denyJoinRequestByInvalidState = deny ? 1 : 0;
}

void Mesh::SetDenyJoinRequestBySessionIsFull(bool deny)
{
    m_denyJoinRequestBySessionIsFull = deny ? 1 : 0;
}

void Mesh::SetupLocalPlayerInfo()
{
    (void)m_localPlayerCount;
}

void Mesh::SetupProtocols()
{
    if (m_meshProtocol != 0)
    {
        struct NativeProtocol { virtual void StartupNative() = 0; };
        reinterpret_cast<NativeProtocol*>(m_meshProtocol)->StartupNative();
    }
}

void Mesh::StartUse(StationIndex index)
{
    const unsigned int value = static_cast<unsigned int>(index);
    if (value < 32)
    {
        const unsigned int mask = 1u << value;
        m_validStationBitmap |= mask;
        if (m_unknown24 != 0)
            *reinterpret_cast<unsigned int*>(static_cast<unsigned char*>(m_unknown24) + 0x54) |= mask;
    }
}

void Mesh::FixConnectedId(StationIndex index)
{
    StartUse(index);
    NoticeMeshEvent(cEventTypeJoin, index);
}

void Mesh::JoinResponseComplete(StationIndex index)
{
    FixConnectedId(index);
}

void Mesh::SetMeshStartTime(const common::Time& time)
{
    m_meshStartTime = time.m_tick;
}

void Mesh::ProcessingAtMeshBegin()
{
    m_meshStartTimeValid = 1;
}

void Mesh::ProcessingAtEndHM()
{
}

void Mesh::NotifyLeaveStationAddress(const common::StationAddress& address)
{
    m_unknownc7 = address.IsValid() ? 1 : 0;
}

void Mesh::AddPlayHistory(transport::Station*, transport::Station::PlayerInfo*, unsigned int count)
{
    (void)count;
}

void Mesh::NoticeMeshEvent(EventType, StationIndex index)
{
    if (static_cast<unsigned int>(index) < 32)
        m_validStationBitmap |= 1u << static_cast<unsigned int>(index);
}

bool Mesh::IsHost() const
{
    return m_unknown6c != 0;
}

unsigned short Mesh::GetStationNum() const
{
    const unsigned int bitmap = IsHost() ? m_validStationBitmap : m_connectedStationBitmap;
    unsigned short count = 1;
    const unsigned int local = static_cast<unsigned int>(m_unknown5c & 0xff);
    for (unsigned int i = 0; i != 32; ++i)
        if (i != local && (bitmap & (1u << i)) != 0)
            ++count;
    return count;
}

bool Mesh::IsMeshStartTimeValid() const
{
    return m_meshStartTimeValid != 0;
}

long long Mesh::CalcDiffWithMeshStartTime(const common::Time& time) const
{
    const unsigned long long start =
        m_meshStartTime;
    return static_cast<long long>(time.m_tick - start);
}

Result Mesh::MonitoringProcess(DisconnectReason reason, unsigned char value)
{
    if (!IsMeshStartTimeValid())
        return 0;
    common::Time now;
    now.SetNow();
    long long elapsed = CalcDiffWithMeshStartTime(now);
    if (reason != static_cast<DisconnectReason>(1))
    {
        const unsigned int mode = m_unknown140;
        reason = static_cast<DisconnectReason>(mode == 2 ? 4 : (mode != 0 ? 5 : reason));
    }
    const int stationCount = GetStationNum() == 0 ? -1 : static_cast<int>(GetStationNum());
    const int localValue = m_unknown80 == 0 ? -1 : static_cast<int>(m_unknown80);
    m_disconnectReason = static_cast<unsigned int>(reason);
    m_unknownc7 = value;
    m_unknown98 = static_cast<unsigned int>(elapsed);
    m_unknown9c = static_cast<unsigned int>(stationCount);
    if (m_processUpdateMeshJob != 0)
    {
        struct MonitorJob { virtual void SetMonitoringData() = 0; };
        reinterpret_cast<MonitorJob*>(m_processUpdateMeshJob)->SetMonitoringData();
    }
    if (m_processHostMigrationJob != 0)
    {
        struct MonitorJob { virtual void SetMonitoringData() = 0; };
        reinterpret_cast<MonitorJob*>(m_processHostMigrationJob)->SetMonitoringData();
    }
    if (common::g_piaJobScheduler != 0) common::g_piaJobScheduler->SetMonitoringData();
    if (transport::g_piaTransport != 0) transport::g_piaTransport->SetMonitoringNetworkRtt(false);
    if (transport::g_piaTransportAnalyzer != 0) transport::g_piaTransportAnalyzer->SetMonitoringData();
    if (transport::g_piaThreadStreamManager != 0) transport::g_piaThreadStreamManager->SetMonitoringData();
    (void)localValue;
    if (m_skipReport == 0 && m_monitoringDataSender != 0)
        m_monitoringDataSender->RequestSendReport(static_cast<common::MonitoringDataSender::DataType>(value));
    if (value == 1) m_meshStartTimeValid = 0;
    return 0;
}

void Mesh::ProcessingAtMeshEnd(DisconnectReason reason)
{
    m_disconnectReason = static_cast<unsigned int>(reason);
    CleanupMonitoringBeginData();
}

Result Mesh::DestroyMesh(common::CallContext* callerContext)
{
    if (m_unknown5c == 0xfd || m_unknown5c != m_unknown58)
        return static_cast<Result>(0x10c08);
    if (callerContext != 0 && callerContext->m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    m_unknown56 = 0;
    if (GetStationNum() < 2)
    {
        m_meshState = cStateStarted;
        ProcessingAtMeshEnd(static_cast<DisconnectReason>(m_disconnectReason));
        Cleanup();
        if (callerContext != 0) callerContext->SignalSuccess(0);
        return 0;
    }
    if (callerContext != 0) callerContext->InitiateCall();
    m_callContext.InitiateCall();
    return 0;
}

bool Mesh::IsHostMigrationEnabled() const
{
    return m_unknown6c != 0;
}

Result Mesh::LeaveMeshWithHostMigration(common::CallContext* callerContext)
{
    if (!IsHostMigrationEnabled() || m_unknown5c == 0xfd || m_unknown5c != m_unknown58)
        return static_cast<Result>(0x10c08);
    if (callerContext != 0 && callerContext->m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    m_unknown56 = 0;
    if (GetStationNum() < 2)
    {
        m_meshState = cStateStarted;
        ProcessingAtMeshEnd(static_cast<DisconnectReason>(m_disconnectReason));
        Cleanup();
        if (callerContext != 0) callerContext->SignalSuccess(0);
        return 0;
    }
    if (callerContext != 0) callerContext->InitiateCall();
    m_callContext.InitiateCall();
    return 0;
}

bool Mesh::IsProcessingLeaveMesh() const
{
    return m_asyncProcess == cAsyncLeaveMesh ||
           m_asyncProcess == cAsyncLeaveMeshWithHostMigration;
}

StationIndex Mesh::GetUsableStationIndex()
{
    for (unsigned int index = 0; index < 32; ++index)
        if ((m_validStationBitmap & (1u << index)) == 0)
            return static_cast<StationIndex>(index);
    return static_cast<StationIndex>(0xfd);
}

Result Mesh::CheckApprovalJoin(transport::Station*)
{
    if (m_unknown56 == 0 || IsProcessingLeaveMesh())
        return static_cast<Result>(2);
    if (m_unknown54 <= GetStationNum() || GetUsableStationIndex() == static_cast<StationIndex>(0xfd))
        return 0;
    if (m_joiningMeshApprovalCallback != 0 &&
        !m_joiningMeshApprovalCallback(0))
        return static_cast<Result>(2);
    if (m_denyJoinRequestBySessionIsFull != 0) { m_denyJoinRequestBySessionIsFull = 0; return 0; }
    if (m_denyJoinRequestByInvalidState != 0) { m_denyJoinRequestByInvalidState = 0; return static_cast<Result>(4); }
    return static_cast<Result>(0xff);
}

Result Mesh::JoinMeshCore(const transport::StationConnectionInfo&,
                          common::CallContext* callerContext)
{
    if (m_meshState == cStateStarted || m_unknown60 != 0 ||
        (callerContext != 0 && callerContext->m_state == common::CallContext::cStateInProgress))
        return static_cast<Result>(0x10c08);
    m_unknown88 = 0;
    m_unknown58 = 0xfe;
    m_unknown80 = 0;
    m_unknowncc = 0;
    m_unknown8c = 0;
    m_unknownc8 = 0;
    if (callerContext != 0) callerContext->InitiateCall();
    m_callContext.InitiateCall();
    return 0;
}

Result Mesh::LeaveMesh(common::CallContext* callerContext)
{
    if (m_unknown5c == 0xfd || m_unknown5c != m_unknown58)
        return static_cast<Result>(0x10c08);
    if (callerContext != 0 && callerContext->m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    m_unknown56 = 0;
    if (GetStationNum() < 2)
    {
        m_meshState = cStateStarted;
        ProcessingAtMeshEnd(static_cast<DisconnectReason>(m_disconnectReason));
        Cleanup();
        if (callerContext != 0) callerContext->SignalSuccess(0);
        return 0;
    }
    if (callerContext != 0) callerContext->InitiateCall();
    m_callContext.InitiateCall();
    return 0;
}

common::Time Mesh::GetTime() const
{
    common::Time time;
    time.m_tick = m_meshStartTime;
    return time;
}

ProcessHostMigrationJob* Mesh::GetProcessHostMigrationJobPtr()
{
    return m_processHostMigrationJob;
}

ProcessDestroyMeshJob* Mesh::GetProcessDestroyMeshJobPtr()
{
    return reinterpret_cast<ProcessDestroyMeshJob*>(m_unknown20);
}

ProcessUpdateMeshJob* Mesh::GetProcessUpdateMeshJobPtr()
{
    return m_processUpdateMeshJob;
}

Result Mesh::setDebugSettingImpl(const DebugSetting& setting)
{
    if (g_piaMesh == 0)
        return static_cast<Result>(0x10c08);
    g_piaMesh->m_unknownc6 = static_cast<unsigned char>(setting.m_field00 != 0);
    g_piaMesh->m_unknownc7 = static_cast<unsigned char>(setting.m_field04 != 0);
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
    return static_cast<Result>(m_disconnectReason);
}

unsigned int Mesh::GetDisconnectReason() const
{
    return m_disconnectReason;
}

bool Mesh::IsBandwidthCheckProtocolSetupParamValid(int measurementTime,
                                                    unsigned int ipPacketSize,
                                                    int measurementCount) const
{
    return measurementTime > 0 && ipPacketSize > 0 && measurementCount > 0;
}

}}}
