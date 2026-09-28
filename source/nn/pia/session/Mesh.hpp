#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_MESH_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_MESH_HPP

#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/MonitoringDataSender.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn { namespace pia { namespace session {

class KickoutManageJob;
class ProcessHostMigrationJob;
class ProcessDestroyMeshJob;
class ProcessUpdateMeshJob;
class CreateMeshJob;
class JoinMeshJob;
class LeaveMeshJob;
class DestroyMeshJob;
class MeshProtocol;
class MeshEventListener;

class Mesh
{
public:
    struct Setting
    {
        unsigned int m_field00;
        unsigned int m_field04;
        unsigned int m_field08;
        unsigned int m_field0c;
        unsigned int m_field10;
    };

    struct StartupSetting
    {
        const transport::Station::PlayerInfo* m_playerInfo;
        unsigned char m_playerCount;
    };

    struct DebugSetting
    {
        unsigned int m_field00;
        unsigned int m_field04;
    };

    enum State
    {
        cStateNone = 0,
        cStateCreated = 1,
        cStateStarted = 2
    };

    enum EventType
    {
        cEventTypeJoin = 0,
        cEventTypeLeave = 1
    };

    enum DisconnectReason
    {
        cDisconnectReasonNone = 0
    };

    enum EmulationType
    {
        cEmulationTypeNone = 0
    };

    typedef bool (*JoiningMeshApprovalCallback)(
        const transport::Station::IdentificationToken*);
    typedef unsigned char (*HostCandidateRankingCallback)(StationIndex, bool);
    typedef void (*MeshEventCallback)(EventType, unsigned long long);
    typedef unsigned int (*GetSessionIdCallback)();

    enum AsyncProcessingName
    {
        cAsyncCreateMesh = 1,
        cAsyncDestroyMesh = 2,
        cAsyncLeaveMeshWithHostMigration = 3,
        cAsyncJoinMesh = 4,
        cAsyncLeaveMesh = 5
    };
    unsigned int GetHostMigrationMode() const;
    bool IsRecentPlayRecordEnabled() const;
    bool IsBandwidthCheckProtocolSetupParamValid(int measurementTime,
                                                 unsigned int ipPacketSize,
                                                 int measurementCount) const;
    void SetSyncClockPulseInterval(int interval);
    void SetSkipCleanupReport(bool skip);
    Result CheckConnectionError() const;
    unsigned int GetDisconnectReason() const;
    KickoutManageJob* GetKickoutManageJobPtr();

    Result JoinMeshAsync(const transport::StationConnectionInfo& info);
    void ResetInternalAsyncState();
    bool IsJoinMeshCompleted() const;
    Result GetJoinMeshResult() const;
    void CancelJoinMeshAsync();
    Result LeaveMeshAsync();
    bool IsLeaveMeshCompleted() const;
    Result GetLeaveMeshResult() const;
    Result LeaveMeshWithHostMigrationAsync();
    bool IsLeaveMeshWithHostMigrationCompleted() const;
    Result GetLeaveMeshWithHostMigrationResult() const;
    Result DestroyMeshAsync();
    bool IsDestroyMeshCompleted() const;
    Result GetDestroyMeshResult() const;
    unsigned int GetJoinMeshState() const;
    void SetSkipReport(bool skip);
    void SendReport(unsigned char reason);
    unsigned int GetMeshState() const;
    StationIndex GetHostStationIndex() const;
    StationIndex GetLocalStationIndex() const;
    void* GetBandwidthCheckProtocol();
    void RegisterJoiningMeshApprovalCallback(JoiningMeshApprovalCallback callback);
    void UnregisterJoiningMeshApprovalCallback();
    void RegisterCheckHostCandidateRankingCallback(HostCandidateRankingCallback callback);
    void UnregisterCheckHostCandidateRankingCallback();
    void RegisterEventListener(MeshEventListener* listener);
    void UnregisterEventListener();
    void RegisterGetSessionIdCallback(GetSessionIdCallback callback);
    void UnregisterGetSessionIdCallback();
    void* GetMeshProtocol();
    void* GetStationProtocol();
    void SetHostMigrationStarted(bool started);
    bool IsHostMigrationStarted();
    void SetFatalError(DisconnectReason reason);
    unsigned int GetNetworkTopologyType();
    void SetupSendReport();
    bool IsProcessedAtMeshBegin() const;
    bool IsProcessingHostMigration() const;
    unsigned int GetLocalPlayerCount() const;
    bool IsPlayersCountedAsParticipants() const;
    void CleanupMonitoringBeginData();
    common::MonitoringDataSender* GetMonitoringDataSender();
    unsigned int GetBitRateCheckMode();
    void SetDenyJoinRequestByInvalidState(bool deny);
    void SetDenyJoinRequestBySessionIsFull(bool deny);
    void SetupLocalPlayerInfo();
    void SetupProtocols();
    void StartUse(StationIndex index);
    void FixConnectedId(StationIndex index);
    void JoinResponseComplete(StationIndex index);
    void SetMeshStartTime(const common::Time& time);
    void ProcessingAtMeshBegin();
    void ProcessingAtEndHM();
    void NotifyLeaveStationAddress(const common::StationAddress& address);
    void AddPlayHistory(transport::Station* station,
                        transport::Station::PlayerInfo* info,
                        unsigned int count);
    void NoticeMeshEvent(EventType type, StationIndex index);
    bool IsHost() const;
    unsigned short GetStationNum() const;
    bool IsMeshStartTimeValid() const;
    long long CalcDiffWithMeshStartTime(const common::Time& time) const;
    Result MonitoringProcess(DisconnectReason reason, unsigned char value);
    void ProcessingAtMeshEnd(DisconnectReason reason);
    Result DestroyMesh(common::CallContext* callerContext);
    bool IsHostMigrationEnabled() const;
    Result LeaveMeshWithHostMigration(common::CallContext* callerContext);
    bool IsProcessingLeaveMesh() const;
    StationIndex GetUsableStationIndex();
    Result CheckApprovalJoin(transport::Station* station);
    Result JoinMeshCore(const transport::StationConnectionInfo& info,
                        common::CallContext* callerContext);
    Result LeaveMesh(common::CallContext* callerContext);
    common::Time GetTime() const;
    ProcessHostMigrationJob* GetProcessHostMigrationJobPtr();
    ProcessDestroyMeshJob* GetProcessDestroyMeshJobPtr();
    ProcessUpdateMeshJob* GetProcessUpdateMeshJobPtr();
    static Result setDebugSettingImpl(const DebugSetting& setting);
    static void SetDebugSettingForInternal(const DebugSetting& setting);
    void TraceStationConnectionInfoList(unsigned long long value);
    void Trace(unsigned long long value) const;

    void JoinMesh(const transport::StationConnectionInfo& info,
                  common::CallContext* callerContext);
    void KickoutStationByIndex(StationIndex index);
    void KickoutStation(unsigned long long stationId);
    unsigned long long GetHostStationId() const;
    unsigned long long GetLocalStationId() const;
    bool IsValidStationId(unsigned long long stationId) const;
    unsigned int GetValidStationIndexBitmap() const;
    bool IsBitRateCheckCompleted() const;
    Result GetUplinkBitRate(int* bitRate);
    void RegisterMeshEventCallback(MeshEventCallback callback);
    void UnregisterMeshEventCallback();
    int GetSyncClockPulseInterval() const;
    void SetMaxNumOfRelayRoutesAssignedToStation(unsigned short value);
    void SetMaxRtt(unsigned short value);
    Result GetConnectionRoute(StationIndex index, StationIndex* route);
    void RelayEmulation(StationIndex from, StationIndex to, EmulationType type);
    void ResetRelayEmulation();
    unsigned int GetHostMigrationSequence() const;
    unsigned int GetHostMigrationState() const;
    unsigned int GetProcessUpdateMeshJobStepSequence() const;
    bool IsMultiPlayerEnabled() const;
    void SetSyncClockBaseTimeForDebug(const common::Time& time);

    Mesh(const Setting& setting);
    ~Mesh();
    Result Initialize(const Setting& setting, bool isHost);
    static Result CreateInstance(const Setting& setting);
    static void DestroyInstance();
    void CleanupStationsJobs();
    void CleanupJobs();
    bool IsValidStationIndex(StationIndex index) const;
    void StopUse(StationIndex index);
    void UnfixDisconnectedId(StationIndex index);
    void ResetStationIndexUsage();
    void SetMeshState(State state);
    void CleanupStatus();
    void Cleanup();
    Result CheckPlayerInfo(const transport::Station::PlayerInfo& info) const;
    Result CheckPlayerInfoSetting(const transport::Station::PlayerInfo* infos,
                                  unsigned char count) const;
    Result Startup(StartupSetting& setting);
    Result CreateMesh(common::CallContext* callerContext);
    Result CreateMeshAsync();
    bool IsAsyncProcessCompleted(AsyncProcessingName process) const;
    bool IsCreateMeshCompleted() const;
    Result GetAsyncProcessResult(AsyncProcessingName process) const;
    Result GetCreateMeshResult() const;
    void CancelAsyncProcess(AsyncProcessingName process);

    CreateMeshJob* m_createMeshJob;
    JoinMeshJob* m_joinMeshJob;
    LeaveMeshJob* m_leaveMeshJob;
    void* m_unknown0c;
    ProcessUpdateMeshJob* m_processUpdateMeshJob;
    unsigned int m_unknown14;
    void* m_unknown18;
    ProcessHostMigrationJob* m_processHostMigrationJob;
    void* m_unknown20;
    void* m_unknown24;
    KickoutManageJob* m_kickoutManageJob;
    transport::ProtocolId m_syncClockProtocolId;
    JoiningMeshApprovalCallback m_joiningMeshApprovalCallback;
    HostCandidateRankingCallback m_hostCandidateRankingCallback;
    GetSessionIdCallback m_getSessionIdCallback;
    unsigned int m_unknown3c;
    MeshProtocol* m_meshProtocol;
    unsigned int m_unknown48;
    unsigned short m_unknown4c;
    unsigned short m_unknown4e;
    void* m_bandwidthCheckProtocol;
    unsigned short m_unknown54;
    unsigned char m_unknown56;
    unsigned char m_unknown57;
    unsigned int m_unknown58;
    unsigned int m_unknown5c;
    unsigned int m_unknown60;
    unsigned int m_meshState;
    unsigned int m_disconnectReason;
    unsigned int m_unknown6c;
    unsigned int m_networkTopologyType;
    unsigned int m_bitRateCheckMode;
    unsigned int m_unknown70;
    unsigned char m_unknown74;
    unsigned char m_unknown75;
    unsigned char m_unknown76;
    unsigned char m_unknown77;
    unsigned int m_connectedStationBitmap;
    unsigned int m_validStationBitmap;
    unsigned int m_unknown80;
    unsigned char m_unknown84[4];
    unsigned int m_unknown88;
    unsigned int m_unknown8c;
    unsigned short m_unknown90;
    unsigned char m_unknown92[6];
    unsigned int m_unknown98;
    unsigned int m_unknown9c;
    unsigned long long m_meshStartTime;
    unsigned char m_meshStartTimeValid;
    unsigned char m_playersCountedAsParticipants;
    unsigned char m_recentPlayRecordEnabled;
    common::MonitoringDataSender* m_monitoringDataSender;
    common::CallContext m_callContext;
    unsigned int m_asyncProcess;
    void* m_meshEventListener;
    unsigned char m_skipReport;
    unsigned char m_skipCleanupReport;
    unsigned char m_unknownc6;
    unsigned char m_unknownc7;
    unsigned int m_unknownc8;
    unsigned char m_unknowncc;
    unsigned char m_denyJoinRequestByInvalidState;
    unsigned char m_denyJoinRequestBySessionIsFull;
    unsigned char m_unknowncf;
    void* m_playerInfoVector;
    unsigned char m_unknownd4[0x64];
    unsigned char m_unknown138;
    unsigned char m_unknown139[7];
    unsigned int m_unknown140;
    unsigned int m_unknown144;
    unsigned int m_unknown148;
    unsigned int m_unknown14c;
    unsigned char m_localPlayerCount;
    unsigned char m_unknown151[7];
};

extern Mesh* g_piaMesh;

#if !defined(RECON_NATIVE)
#endif

}}}

#endif
