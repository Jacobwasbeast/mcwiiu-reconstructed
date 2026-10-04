#include "nn/pia/inet/NexNetworkFactory.hpp"
#include "nn/pia/common/MonitoringDataSender.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/inet/MonitoringDataSender.hpp"
#include "nn/pia/session/ICommunityInfoList.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/ISessionInfoList.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/String.hpp"
#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/MissingStationHandler.hpp"
#include "nn/pia/transport/RelayStationPacketHandler.hpp"

namespace nn { namespace pia { namespace inet {

namespace {

enum { cMaxStationNum = 16, cHeaderSize = 28, cMtuSizeMax = 1364, cMtuSizeMin = 576 };

}

NexNetworkFactory::NexNetworkFactory()
{
}

NexNetworkFactory::~NexNetworkFactory()
{
}

unsigned int NexNetworkFactory::GetMaxStationNum() const
{
    return cMaxStationNum;
}

unsigned int NexNetworkFactory::GetHeaderSize() const
{
    return cHeaderSize;
}

unsigned int NexNetworkFactory::GetMtuSizeMax() const
{
    return cMtuSizeMax;
}

unsigned int NexNetworkFactory::GetMtuSizeMin() const
{
    return cMtuSizeMin;
}

int NexNetworkFactory::GetDefaultRttInitialPulseInterval() const
{
    return 200;
}

bool NexNetworkFactory::IsClientRelayConnectionPermitted() const
{
    return true;
}

bool NexNetworkFactory::IsBitRateCheckPermitted() const
{
    return true;
}

void NexNetworkFactory::DeleteSelf(transport::NetworkFactory* factory)
{
    transport::DestroyObject(factory);
}

transport::PacketHandler* NexNetworkFactory::CreatePacketHandler()
{
    return transport::CreateObject<transport::RelayStationPacketHandler>();
}

void NexNetworkFactory::DeletePacketHandler(transport::PacketHandler* handler)
{
    transport::DestroyObject(handler);
}

void NexNetworkFactory::DeleteMissingStationHandler(transport::MissingStationHandler* handler)
{
    transport::DestroyObject(handler);
}

void NexNetworkFactory::DeleteConnectStationJob(transport::ConnectStationJob* job)
{
    transport::DestroyObject(job);
}

void NexNetworkFactory::DeleteDisconnectStationJob(transport::DisconnectStationJob* job)
{
    transport::DestroyObject(job);
}

common::MonitoringDataSender* NexNetworkFactory::CreateMonitoringDataSender()
{
    return transport::CreateObject<MonitoringDataSender>();
}

void NexNetworkFactory::DeleteMonitoringDataSender(common::MonitoringDataSender* sender)
{
    transport::DestroyObject(sender);
}

unsigned int NexNetworkFactory::GetHostMigrationMode() const
{
    return 2;
}

unsigned int NexNetworkFactory::GetBrowseSessionInfoListSizeMax() const
{
    return 300;
}

unsigned int NexNetworkFactory::GetBrowseCommunityInfoListSizeMax() const
{
    return 100;
}

bool NexNetworkFactory::IsBindRequired() const
{
    return true;
}

bool NexNetworkFactory::IsServerRequired() const
{
    return true;
}

bool NexNetworkFactory::IsMultiCandidatePermitted() const
{
    return true;
}

bool NexNetworkFactory::IsJointSessionPermitted() const
{
    return true;
}

bool NexNetworkFactory::IsMultiplayerPermitted() const
{
    return true;
}

bool NexNetworkFactory::IsRecentPlayRecordPermitted() const
{
    return true;
}

void NexNetworkFactory::DeleteCreateMeshJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteJoinMeshJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteLeaveMeshJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteProcessHostMigrationJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteLeaveWithHostMigrationJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteKickoutManageJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteMatchCreateSessionJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteMatchRandomMatchmakeJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteMatchBrowseMatchmakeJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteMatchJoinSessionJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteMatchLeaveSessionJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteMatchDestroySessionJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DestroyMatchJointSessionJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteMatchUpdateSessionSettingJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteGenerateMatchmakeSystemPasswordJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteClearMatchmakeSystemPasswordJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteCommunityManagementJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteSessionInfoList(session::ISessionInfoList* list)
{
    transport::DestroyObject(list);
}

void NexNetworkFactory::DeleteCommunityInfoList(session::ICommunityInfoList* list)
{
    transport::DestroyObject(list);
}

void NexNetworkFactory::DeleteMatchmakeSession(session::IMatchmakeSession* session)
{
    transport::DestroyObject(session);
}

void NexNetworkFactory::DeleteMatchmakeSessionSystemPassword(common::String* password)
{
    transport::DestroyObject(password);
}

void NexNetworkFactory::DeleteMatchMeshLayerController(session::MeshLayerController* controller)
{
    transport::DestroyObject(controller);
}

}}}
