#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNETWORKFACTORY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNETWORKFACTORY_HPP

#include <stdint.h>

namespace nn { namespace pia {

namespace common {
struct IPacketInput;
struct IPacketOutput;
struct MonitoringDataSender;
struct StepSequenceJob;
struct String;
}

namespace session {
struct ICommunityInfoList;
struct IMatchmakeSession;
struct ISessionInfoList;
struct MeshLayerController;
}

namespace transport {
struct ConnectStationJob;
struct DisconnectStationJob;
struct MissingStationHandler;
struct NetworkFactory;
struct PacketHandler;
}

namespace inet {

class NexNetworkFactory
{
public:
    NexNetworkFactory();
    ~NexNetworkFactory();
    uint32_t CreateSelf();
    uint64_t DeleteSelf(transport::NetworkFactory* a1);
    uint64_t CreateProtocols();
    uint32_t CreatePacketHandler();
    uint32_t CreateMissingStationHandler();
    uint64_t DeletePacketHandler(transport::PacketHandler* a1);
    uint64_t DeleteMissingStationHandler(transport::MissingStationHandler* a1);
    int32_t CreateInputStream();
    int32_t CreateOutputStream();
    uint64_t DeleteInputStream(common::IPacketInput* a1);
    uint64_t DeleteOutputStream(common::IPacketOutput* a1);
    uint32_t CreateConnectStationJob();
    uint32_t CreateDisconnectStationJob();
    uint64_t DeleteConnectStationJob(transport::ConnectStationJob* a1);
    uint64_t DeleteDisconnectStationJob(transport::DisconnectStationJob* a1);
    uint32_t CreateCreateMeshJob();
    uint32_t CreateJoinMeshJob();
    uint32_t CreateLeaveMeshJob();
    uint32_t CreateProcessHostMigrationJob();
    uint32_t CreateLeaveWithHostMigrationJob();
    uint32_t CreateKickoutManageJob();
    uint64_t DeleteCreateMeshJob(common::StepSequenceJob* a1);
    uint64_t DeleteJoinMeshJob(common::StepSequenceJob* a1);
    uint64_t DeleteLeaveMeshJob(common::StepSequenceJob* a1);
    uint64_t DeleteProcessHostMigrationJob(common::StepSequenceJob* a1);
    uint64_t DeleteLeaveWithHostMigrationJob(common::StepSequenceJob* a1);
    uint64_t DeleteKickoutManageJob(common::StepSequenceJob* a1);
    uint32_t CreateMatchCreateSessionJob();
    uint32_t CreateMatchRandomMatchmakeJob();
    uint32_t CreateMatchBrowseMatchmakeJob();
    uint32_t CreateMatchJoinSessionJob();
    uint32_t CreateMatchLeaveSessionJob();
    uint32_t CreateMatchDestroySessionJob();
    uint32_t CreateMatchUpdateSessionSettingJob();
    uint32_t CreateMatchJointSessionJob();
    uint64_t DeleteMatchCreateSessionJob(common::StepSequenceJob* a1);
    uint64_t DeleteMatchRandomMatchmakeJob(common::StepSequenceJob* a1);
    uint64_t DeleteMatchBrowseMatchmakeJob(common::StepSequenceJob* a1);
    uint64_t DeleteMatchJoinSessionJob(common::StepSequenceJob* a1);
    uint64_t DeleteMatchLeaveSessionJob(common::StepSequenceJob* a1);
    uint64_t DeleteMatchDestroySessionJob(common::StepSequenceJob* a1);
    uint64_t DeleteMatchUpdateSessionSettingJob(common::StepSequenceJob* a1);
    uint64_t DestroyMatchJointSessionJob(common::StepSequenceJob* a1);
    uint32_t CreateGenerateMatchmakeSystemPasswordJob();
    uint32_t CreateClearMatchmakeSystemPasswordJob();
    uint32_t CreateCommunityManagementJob();
    uint64_t DeleteGenerateMatchmakeSystemPasswordJob(common::StepSequenceJob* a1);
    uint64_t DeleteClearMatchmakeSystemPasswordJob(common::StepSequenceJob* a1);
    uint64_t DeleteCommunityManagementJob(common::StepSequenceJob* a1);
    uint32_t CreateSessionInfoList(uint16_t a1);
    uint32_t CreateCommunityInfoList(uint8_t a1);
    uint32_t CreateMatchmakeSession();
    uint32_t CreateMatchmakeSessionSystemPassword();
    uint32_t CreateMatchMeshLayerController();
    uint64_t DeleteSessionInfoList(session::ISessionInfoList* a1);
    uint64_t DeleteCommunityInfoList(session::ICommunityInfoList* a1);
    uint64_t DeleteMatchmakeSession(session::IMatchmakeSession* a1);
    uint64_t DeleteMatchmakeSessionSystemPassword(common::String* a1);
    uint64_t DeleteMatchMeshLayerController(session::MeshLayerController* a1);
    uint32_t CreateMonitoringDataSender();
    uint64_t DeleteMonitoringDataSender(common::MonitoringDataSender* a1);
    uint32_t GetMaxStationNum() const;
    uint32_t GetHeaderSize() const;
    uint32_t GetHostMigrationMode() const;
    uint32_t GetBrowseSessionInfoListSizeMax() const;
    uint32_t GetBrowseCommunityInfoListSizeMax() const;
    uint32_t GetMtuSizeMax() const;
    uint32_t GetMtuSizeMin() const;
    uint32_t GetDefaultRttInitialPulseInterval() const;
    uint32_t IsBindRequired() const;
    uint32_t IsServerRequired() const;
    uint32_t IsMultiCandidatePermitted() const;
    uint32_t IsClientRelayConnectionPermitted() const;
    uint32_t IsJointSessionPermitted() const;
    uint32_t IsMultiplayerPermitted() const;
    uint32_t IsRecentPlayRecordPermitted() const;
    uint32_t IsBitRateCheckPermitted() const;

private:
    uint64_t state_;
};

}}}

#endif
