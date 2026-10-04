#include "nn/pia/inet/NexNetworkFactory.hpp"

namespace nn { namespace pia { namespace inet {

uint32_t NexNetworkFactory::CreateSelf()
{
    return static_cast<uint32_t>(state_);
}

uint64_t NexNetworkFactory::DeleteSelf(transport::NetworkFactory* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::CreateProtocols()
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNetworkFactory::CreatePacketHandler()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateMissingStationHandler()
{
    return static_cast<uint32_t>(state_);
}

uint64_t NexNetworkFactory::DeletePacketHandler(transport::PacketHandler* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMissingStationHandler(transport::MissingStationHandler* a1)
{
    return static_cast<uint64_t>(state_);
}

int32_t NexNetworkFactory::CreateInputStream()
{
    return static_cast<int32_t>(state_);
}

int32_t NexNetworkFactory::CreateOutputStream()
{
    return static_cast<int32_t>(state_);
}

uint64_t NexNetworkFactory::DeleteInputStream(common::IPacketInput* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteOutputStream(common::IPacketOutput* a1)
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNetworkFactory::CreateConnectStationJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateDisconnectStationJob()
{
    return static_cast<uint32_t>(state_);
}

uint64_t NexNetworkFactory::DeleteConnectStationJob(transport::ConnectStationJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteDisconnectStationJob(transport::DisconnectStationJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNetworkFactory::CreateCreateMeshJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateJoinMeshJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateLeaveMeshJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateProcessHostMigrationJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateLeaveWithHostMigrationJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateKickoutManageJob()
{
    return static_cast<uint32_t>(state_);
}

uint64_t NexNetworkFactory::DeleteCreateMeshJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteJoinMeshJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteLeaveMeshJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteProcessHostMigrationJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteLeaveWithHostMigrationJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteKickoutManageJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNetworkFactory::CreateMatchCreateSessionJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateMatchRandomMatchmakeJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateMatchBrowseMatchmakeJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateMatchJoinSessionJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateMatchLeaveSessionJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateMatchDestroySessionJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateMatchUpdateSessionSettingJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateMatchJointSessionJob()
{
    return static_cast<uint32_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchCreateSessionJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchRandomMatchmakeJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchBrowseMatchmakeJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchJoinSessionJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchLeaveSessionJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchDestroySessionJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchUpdateSessionSettingJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DestroyMatchJointSessionJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNetworkFactory::CreateGenerateMatchmakeSystemPasswordJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateClearMatchmakeSystemPasswordJob()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateCommunityManagementJob()
{
    return static_cast<uint32_t>(state_);
}

uint64_t NexNetworkFactory::DeleteGenerateMatchmakeSystemPasswordJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteClearMatchmakeSystemPasswordJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteCommunityManagementJob(common::StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNetworkFactory::CreateSessionInfoList(uint16_t a1)
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateCommunityInfoList(uint8_t a1)
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateMatchmakeSession()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateMatchmakeSessionSystemPassword()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::CreateMatchMeshLayerController()
{
    return static_cast<uint32_t>(state_);
}

uint64_t NexNetworkFactory::DeleteSessionInfoList(session::ISessionInfoList* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteCommunityInfoList(session::ICommunityInfoList* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchmakeSession(session::IMatchmakeSession* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchmakeSessionSystemPassword(common::String* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchMeshLayerController(session::MeshLayerController* a1)
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNetworkFactory::CreateMonitoringDataSender()
{
    return static_cast<uint32_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMonitoringDataSender(common::MonitoringDataSender* a1)
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNetworkFactory::GetMaxStationNum() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::GetHeaderSize() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::GetHostMigrationMode() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::GetBrowseSessionInfoListSizeMax() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::GetBrowseCommunityInfoListSizeMax() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::GetMtuSizeMax() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::GetMtuSizeMin() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::GetDefaultRttInitialPulseInterval() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsBindRequired() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsServerRequired() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsMultiCandidatePermitted() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsClientRelayConnectionPermitted() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsJointSessionPermitted() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsMultiplayerPermitted() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsRecentPlayRecordPermitted() const
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsBitRateCheckPermitted() const
{
    return static_cast<uint32_t>(state_);
}

}}}
