#include "nn/pia/session/SessionNetworkFactory.hpp"

#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace session {

SessionNetworkFactory::SessionNetworkFactory() : transport::NetworkFactory() {}
SessionNetworkFactory::~SessionNetworkFactory() {}
bool SessionNetworkFactory::IsClientRelayConnectionPermitted() const { return false; }
bool SessionNetworkFactory::IsBitRateCheckPermitted() const { return false; }
void SessionNetworkFactory::CreateProtocols() {}
transport::PacketHandler* SessionNetworkFactory::CreatePacketHandler()
{
    void* storage = pead::allocateFromSeadHeap(sizeof(transport::StationPacketHandler), common::HeapManager::GetHeap());
    if (!storage) return 0;
    return new (storage) transport::StationPacketHandler();
}
transport::MissingStationHandler* SessionNetworkFactory::CreateMissingStationHandler() { return 0; }
void SessionNetworkFactory::DeletePacketHandler(transport::PacketHandler* handler)
{
    if (!handler) return;
    handler->~PacketHandler();
    pead::freeToSeadHeap(handler);
}
void SessionNetworkFactory::DeleteMissingStationHandler(transport::MissingStationHandler*) {}
transport::ConnectStationJob* SessionNetworkFactory::CreateConnectStationJob() { return 0; }
transport::DisconnectStationJob* SessionNetworkFactory::CreateDisconnectStationJob()
{
    void* storage = pead::allocateFromSeadHeap(sizeof(transport::DisconnectStationJob), common::HeapManager::GetHeap());
    if (!storage) return 0;
    return new (storage) transport::DisconnectStationJob();
}
void SessionNetworkFactory::DeleteConnectStationJob(transport::ConnectStationJob*) {}
void SessionNetworkFactory::DeleteDisconnectStationJob(transport::DisconnectStationJob* job)
{
    if (!job) return;
    job->~DisconnectStationJob();
    pead::freeToSeadHeap(job);
}
void SessionNetworkFactory::DeleteGenerateMatchmakeSystemPasswordJob(common::StepSequenceJob*) {}
void SessionNetworkFactory::DeleteClearMatchmakeSystemPasswordJob(common::StepSequenceJob*) {}
void SessionNetworkFactory::DeleteCommunityManagementJob(common::StepSequenceJob*) {}
ICommunityInfoList* SessionNetworkFactory::CreateCommunityInfoList(unsigned char) { return 0; }
void SessionNetworkFactory::DeleteCommunityInfoList(ICommunityInfoList*) {}
void SessionNetworkFactory::DeleteMatchmakeSessionSystemPassword(common::String*) {}
common::MonitoringDataSender* SessionNetworkFactory::CreateMonitoringDataSender() { return 0; }
void SessionNetworkFactory::DeleteMonitoringDataSender(common::MonitoringDataSender*) {}
bool SessionNetworkFactory::IsBindRequired() const { return false; }
bool SessionNetworkFactory::IsServerRequired() const { return false; }
bool SessionNetworkFactory::IsMultiCandidatePermitted() const { return false; }
bool SessionNetworkFactory::IsJointSessionPermitted() const { return false; }
bool SessionNetworkFactory::IsMultiplayerPermitted() const { return false; }
bool SessionNetworkFactory::IsRecentPlayRecordPermitted() const { return false; }
common::StepSequenceJob* SessionNetworkFactory::CreateGenerateMatchmakeSystemPasswordJob() { return 0; }
common::StepSequenceJob* SessionNetworkFactory::CreateClearMatchmakeSystemPasswordJob() { return 0; }
common::StepSequenceJob* SessionNetworkFactory::CreateCommunityManagementJob() { return 0; }
common::String* SessionNetworkFactory::CreateMatchmakeSessionSystemPassword() { return 0; }

}}}
