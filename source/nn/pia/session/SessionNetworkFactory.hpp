#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSIONNETWORKFACTORY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSIONNETWORKFACTORY_HPP

#include <stdint.h>

#include "nn/pia/NetworkFactory.hpp"

namespace nn { namespace pia { namespace common { class MonitoringDataSender; class StepSequenceJob; class String; } namespace transport { class PacketHandler; class MissingStationHandler; class ConnectStationJob; class DisconnectStationJob; } namespace session {
class ICommunityInfoList;

class SessionNetworkFactory : public transport::NetworkFactory
{
public:
    SessionNetworkFactory();
    virtual ~SessionNetworkFactory();
    virtual bool IsClientRelayConnectionPermitted() const;
    virtual bool IsBitRateCheckPermitted() const;
    virtual void CreateProtocols();
    virtual transport::PacketHandler* CreatePacketHandler();
    virtual transport::MissingStationHandler* CreateMissingStationHandler();
    virtual void DeletePacketHandler(transport::PacketHandler*);
    virtual void DeleteMissingStationHandler(transport::MissingStationHandler*);
    virtual transport::ConnectStationJob* CreateConnectStationJob();
    virtual transport::DisconnectStationJob* CreateDisconnectStationJob();
    virtual void DeleteConnectStationJob(transport::ConnectStationJob*);
    virtual void DeleteDisconnectStationJob(transport::DisconnectStationJob*);
    virtual void DeleteGenerateMatchmakeSystemPasswordJob(common::StepSequenceJob*);
    virtual void DeleteClearMatchmakeSystemPasswordJob(common::StepSequenceJob*);
    virtual void DeleteCommunityManagementJob(common::StepSequenceJob*);
    virtual ICommunityInfoList* CreateCommunityInfoList(unsigned char);
    virtual void DeleteCommunityInfoList(ICommunityInfoList*);
    virtual void DeleteMatchmakeSessionSystemPassword(common::String*);
    virtual common::MonitoringDataSender* CreateMonitoringDataSender();
    virtual void DeleteMonitoringDataSender(common::MonitoringDataSender*);
    virtual bool IsBindRequired() const;
    virtual bool IsServerRequired() const;
    virtual bool IsMultiCandidatePermitted() const;
    virtual bool IsJointSessionPermitted() const;
    virtual bool IsMultiplayerPermitted() const;
    virtual bool IsRecentPlayRecordPermitted() const;
    virtual common::StepSequenceJob* CreateGenerateMatchmakeSystemPasswordJob();
    virtual common::StepSequenceJob* CreateClearMatchmakeSystemPasswordJob();
    virtual common::StepSequenceJob* CreateCommunityManagementJob();
    virtual common::String* CreateMatchmakeSessionSystemPassword();
};

}}}
#endif
