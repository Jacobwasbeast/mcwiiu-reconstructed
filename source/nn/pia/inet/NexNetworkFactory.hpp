#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNETWORKFACTORY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNETWORKFACTORY_HPP

#include "nn/pia/session/SessionNetworkFactory.hpp"

namespace nn { namespace pia { namespace inet {

class NexNetworkFactory : public session::SessionNetworkFactory
{
public:
    NexNetworkFactory();
    virtual ~NexNetworkFactory();
    virtual unsigned int GetMaxStationNum() const;
    virtual unsigned int GetHeaderSize() const;
    virtual unsigned int GetMtuSizeMax() const;
    virtual unsigned int GetMtuSizeMin() const;
    virtual int GetDefaultRttInitialPulseInterval() const;
    virtual bool IsClientRelayConnectionPermitted() const;
    virtual bool IsBitRateCheckPermitted() const;
    virtual void DeleteSelf(transport::NetworkFactory* factory);
    virtual transport::PacketHandler* CreatePacketHandler();
    virtual void DeletePacketHandler(transport::PacketHandler* handler);
    virtual void DeleteMissingStationHandler(transport::MissingStationHandler* handler);
    virtual void DeleteConnectStationJob(transport::ConnectStationJob* job);
    virtual void DeleteDisconnectStationJob(transport::DisconnectStationJob* job);
    virtual common::MonitoringDataSender* CreateMonitoringDataSender();
    virtual void DeleteMonitoringDataSender(common::MonitoringDataSender* sender);
    virtual unsigned int GetHostMigrationMode() const;
    virtual unsigned int GetBrowseSessionInfoListSizeMax() const;
    virtual unsigned int GetBrowseCommunityInfoListSizeMax() const;
    virtual bool IsBindRequired() const;
    virtual bool IsServerRequired() const;
    virtual bool IsMultiCandidatePermitted() const;
    virtual bool IsJointSessionPermitted() const;
    virtual bool IsMultiplayerPermitted() const;
    virtual bool IsRecentPlayRecordPermitted() const;
    virtual void DeleteCreateMeshJob(common::StepSequenceJob* job);
    virtual void DeleteJoinMeshJob(common::StepSequenceJob* job);
    virtual void DeleteLeaveMeshJob(common::StepSequenceJob* job);
    virtual void DeleteProcessHostMigrationJob(common::StepSequenceJob* job);
    virtual void DeleteLeaveWithHostMigrationJob(common::StepSequenceJob* job);
    virtual void DeleteKickoutManageJob(common::StepSequenceJob* job);
    virtual void DeleteMatchCreateSessionJob(common::StepSequenceJob* job);
    virtual void DeleteMatchRandomMatchmakeJob(common::StepSequenceJob* job);
    virtual void DeleteMatchBrowseMatchmakeJob(common::StepSequenceJob* job);
    virtual void DeleteMatchJoinSessionJob(common::StepSequenceJob* job);
    virtual void DeleteMatchLeaveSessionJob(common::StepSequenceJob* job);
    virtual void DeleteMatchDestroySessionJob(common::StepSequenceJob* job);
    virtual void DestroyMatchJointSessionJob(common::StepSequenceJob* job);
    virtual void DeleteMatchUpdateSessionSettingJob(common::StepSequenceJob* job);
    virtual void DeleteGenerateMatchmakeSystemPasswordJob(common::StepSequenceJob* job);
    virtual void DeleteClearMatchmakeSystemPasswordJob(common::StepSequenceJob* job);
    virtual void DeleteCommunityManagementJob(common::StepSequenceJob* job);
    virtual void DeleteSessionInfoList(session::ISessionInfoList* list);
    virtual void DeleteCommunityInfoList(session::ICommunityInfoList* list);
    virtual void DeleteMatchmakeSession(session::IMatchmakeSession* session);
    virtual void DeleteMatchmakeSessionSystemPassword(common::String* password);
    virtual void DeleteMatchMeshLayerController(session::MeshLayerController* controller);
};

}}}

#endif
