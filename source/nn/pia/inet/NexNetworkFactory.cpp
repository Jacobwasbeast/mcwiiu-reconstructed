#include "nn/pia/inet/NexNetworkFactory.hpp"

namespace nn { namespace pia { namespace inet {

uint32_t NexNetworkFactory::CreateSelf()
{
    return static_cast<uint32_t>(state_);
}

uint64_t NexNetworkFactory::DeleteSelf(NetworkFactory* a1)
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

uint64_t NexNetworkFactory::DeletePacketHandler(PacketHandler* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMissingStationHandler(MissingStationHandler* a1)
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

uint64_t NexNetworkFactory::DeleteInputStream(IPacketInput* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteOutputStream(IPacketOutput* a1)
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

uint64_t NexNetworkFactory::DeleteConnectStationJob(ConnectStationJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteDisconnectStationJob(DisconnectStationJob* a1)
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

uint64_t NexNetworkFactory::DeleteCreateMeshJob(StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteJoinMeshJob(StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteLeaveMeshJob(StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteProcessHostMigrationJob(StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteLeaveWithHostMigrationJob(StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteKickoutManageJob(StepSequenceJob* a1)
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

uint64_t NexNetworkFactory::DeleteMatchCreateSessionJob(StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchRandomMatchmakeJob(StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchBrowseMatchmakeJob(StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchJoinSessionJob(StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchLeaveSessionJob(StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchDestroySessionJob(StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchUpdateSessionSettingJob(StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DestroyMatchJointSessionJob(StepSequenceJob* a1)
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

uint64_t NexNetworkFactory::DeleteGenerateMatchmakeSystemPasswordJob(StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteClearMatchmakeSystemPasswordJob(StepSequenceJob* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteCommunityManagementJob(StepSequenceJob* a1)
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

uint64_t NexNetworkFactory::DeleteSessionInfoList(ISessionInfoList* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteCommunityInfoList(ICommunityInfoList* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchmakeSession(IMatchmakeSession* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchmakeSessionSystemPassword(String* a1)
{
    return static_cast<uint64_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMatchMeshLayerController(MeshLayerController* a1)
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNetworkFactory::CreateMonitoringDataSender()
{
    return static_cast<uint32_t>(state_);
}

uint64_t NexNetworkFactory::DeleteMonitoringDataSender(MonitoringDataSender* a1)
{
    return static_cast<uint64_t>(state_);
}

uint32_t NexNetworkFactory::GetMaxStationNum()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::GetHeaderSize()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::GetHostMigrationMode()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::GetBrowseSessionInfoListSizeMax()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::GetBrowseCommunityInfoListSizeMax()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::GetMtuSizeMax()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::GetMtuSizeMin()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::GetDefaultRttInitialPulseInterval()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsBindRequired()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsServerRequired()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsMultiCandidatePermitted()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsClientRelayConnectionPermitted()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsJointSessionPermitted()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsMultiplayerPermitted()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsRecentPlayRecordPermitted()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexNetworkFactory::IsBitRateCheckPermitted()
{
    return static_cast<uint32_t>(state_);
}

}}}
