#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNETWORKFACTORY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNETWORKFACTORY_HPP

#include <cstdint>

namespace nn { namespace pia { namespace inet {
struct ConnectStationJob;
struct DisconnectStationJob;
struct ICommunityInfoList;
struct IMatchmakeSession;
struct IPacketInput;
struct IPacketOutput;
struct ISessionInfoList;
struct MeshLayerController;
struct MissingStationHandler;
struct MonitoringDataSender;
struct NetworkFactory;
struct PacketHandler;
struct StepSequenceJob;
struct String;

class NexNetworkFactory
{
public:
    NexNetworkFactory();
    ~NexNetworkFactory();
    uint32_t CreateSelf();
    uint64_t DeleteSelf(NetworkFactory* a1);
    uint64_t CreateProtocols();
    uint32_t CreatePacketHandler();
    uint32_t CreateMissingStationHandler();
    uint64_t DeletePacketHandler(PacketHandler* a1);
    uint64_t DeleteMissingStationHandler(MissingStationHandler* a1);
    int32_t CreateInputStream();
    int32_t CreateOutputStream();
    uint64_t DeleteInputStream(IPacketInput* a1);
    uint64_t DeleteOutputStream(IPacketOutput* a1);
    uint32_t CreateConnectStationJob();
    uint32_t CreateDisconnectStationJob();
    uint64_t DeleteConnectStationJob(ConnectStationJob* a1);
    uint64_t DeleteDisconnectStationJob(DisconnectStationJob* a1);
    uint32_t CreateCreateMeshJob();
    uint32_t CreateJoinMeshJob();
    uint32_t CreateLeaveMeshJob();
    uint32_t CreateProcessHostMigrationJob();
    uint32_t CreateLeaveWithHostMigrationJob();
    uint32_t CreateKickoutManageJob();
    uint64_t DeleteCreateMeshJob(StepSequenceJob* a1);
    uint64_t DeleteJoinMeshJob(StepSequenceJob* a1);
    uint64_t DeleteLeaveMeshJob(StepSequenceJob* a1);
    uint64_t DeleteProcessHostMigrationJob(StepSequenceJob* a1);
    uint64_t DeleteLeaveWithHostMigrationJob(StepSequenceJob* a1);
    uint64_t DeleteKickoutManageJob(StepSequenceJob* a1);
    uint32_t CreateMatchCreateSessionJob();
    uint32_t CreateMatchRandomMatchmakeJob();
    uint32_t CreateMatchBrowseMatchmakeJob();
    uint32_t CreateMatchJoinSessionJob();
    uint32_t CreateMatchLeaveSessionJob();
    uint32_t CreateMatchDestroySessionJob();
    uint32_t CreateMatchUpdateSessionSettingJob();
    uint32_t CreateMatchJointSessionJob();
    uint64_t DeleteMatchCreateSessionJob(StepSequenceJob* a1);
    uint64_t DeleteMatchRandomMatchmakeJob(StepSequenceJob* a1);
    uint64_t DeleteMatchBrowseMatchmakeJob(StepSequenceJob* a1);
    uint64_t DeleteMatchJoinSessionJob(StepSequenceJob* a1);
    uint64_t DeleteMatchLeaveSessionJob(StepSequenceJob* a1);
    uint64_t DeleteMatchDestroySessionJob(StepSequenceJob* a1);
    uint64_t DeleteMatchUpdateSessionSettingJob(StepSequenceJob* a1);
    uint64_t DestroyMatchJointSessionJob(StepSequenceJob* a1);
    uint32_t CreateGenerateMatchmakeSystemPasswordJob();
    uint32_t CreateClearMatchmakeSystemPasswordJob();
    uint32_t CreateCommunityManagementJob();
    uint64_t DeleteGenerateMatchmakeSystemPasswordJob(StepSequenceJob* a1);
    uint64_t DeleteClearMatchmakeSystemPasswordJob(StepSequenceJob* a1);
    uint64_t DeleteCommunityManagementJob(StepSequenceJob* a1);
    uint32_t CreateSessionInfoList(uint16_t a1);
    uint32_t CreateCommunityInfoList(uint8_t a1);
    uint32_t CreateMatchmakeSession();
    uint32_t CreateMatchmakeSessionSystemPassword();
    uint32_t CreateMatchMeshLayerController();
    uint64_t DeleteSessionInfoList(ISessionInfoList* a1);
    uint64_t DeleteCommunityInfoList(ICommunityInfoList* a1);
    uint64_t DeleteMatchmakeSession(IMatchmakeSession* a1);
    uint64_t DeleteMatchmakeSessionSystemPassword(String* a1);
    uint64_t DeleteMatchMeshLayerController(MeshLayerController* a1);
    uint32_t CreateMonitoringDataSender();
    uint64_t DeleteMonitoringDataSender(MonitoringDataSender* a1);
    uint32_t GetMaxStationNum();
    uint32_t GetHeaderSize();
    uint32_t GetHostMigrationMode();
    uint32_t GetBrowseSessionInfoListSizeMax();
    uint32_t GetBrowseCommunityInfoListSizeMax();
    uint32_t GetMtuSizeMax();
    uint32_t GetMtuSizeMin();
    uint32_t GetDefaultRttInitialPulseInterval();
    uint32_t IsBindRequired();
    uint32_t IsServerRequired();
    uint32_t IsMultiCandidatePermitted();
    uint32_t IsClientRelayConnectionPermitted();
    uint32_t IsJointSessionPermitted();
    uint32_t IsMultiplayerPermitted();
    uint32_t IsRecentPlayRecordPermitted();
    uint32_t IsBitRateCheckPermitted();

private:
    std::uint64_t state_;
};

}}}

#endif
