#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANNETWORKFACTORY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANNETWORKFACTORY_HPP

#include <stdint.h>
#include "nn/pia/session/SessionNetworkFactory.hpp"
#include "nn/pia/lan/LanConnectStationJob.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class IPacketInput;
class IPacketOutput;
class StepSequenceJob;
}
namespace transport
{
class ConnectStationJob;
class NetworkFactory;
}
namespace session
{
class ISessionInfoList;
class IMatchmakeSession;
class MeshLayerController;
class CreateMeshJob;
class JoinMeshJob;
class LeaveMeshJob;
class KickoutManageJob;
}
namespace lan
{
class LanInputStream;
class LanOutputStream;
class LanConnectStationJob;
class LanLeaveWithHostMigrationJob;
class LanMatchCreateSessionJob;
class LanMatchRandomMatchmakeJob;
class LanMatchBrowseMatchmakeJob;
class LanMatchJoinSessionJob;
class LanMatchLeaveSessionJob;
class LanMatchDestroySessionJob;
class LanMatchUpdateSessionSettingJob;
class LanMatchJointSessionJob;
class LanMatchmakeSession;
class LanMatchMeshLayerController;
class LanNetworkFactory : public session::SessionNetworkFactory
{
public:
    LanNetworkFactory();
    virtual ~LanNetworkFactory();
    uint32_t CreateSelf();
    uint64_t DeleteSelf(transport::NetworkFactory*);
    void CreateProtocols();
    LanInputStream* CreateInputStream();
    LanOutputStream* CreateOutputStream();
    void DeleteInputStream(LanInputStream*);
    void DeleteOutputStream(LanOutputStream*);
    LanConnectStationJob* CreateConnectStationJob();
    void DeleteConnectStationJob(transport::ConnectStationJob*);
    session::CreateMeshJob* CreateCreateMeshJob();
    session::JoinMeshJob* CreateJoinMeshJob();
    session::LeaveMeshJob* CreateLeaveMeshJob();
    uint32_t CreateProcessHostMigrationJob();
    LanLeaveWithHostMigrationJob* CreateLeaveWithHostMigrationJob();
    session::KickoutManageJob* CreateKickoutManageJob();
    void DeleteCreateMeshJob(session::CreateMeshJob*);
    void DeleteJoinMeshJob(session::JoinMeshJob*);
    void DeleteLeaveMeshJob(session::LeaveMeshJob*);
    uint64_t DeleteProcessHostMigrationJob(common::StepSequenceJob*);
    void DeleteLeaveWithHostMigrationJob(LanLeaveWithHostMigrationJob*);
    void DeleteKickoutManageJob(session::KickoutManageJob*);
    LanMatchCreateSessionJob* CreateMatchCreateSessionJob();
    LanMatchRandomMatchmakeJob* CreateMatchRandomMatchmakeJob();
    LanMatchBrowseMatchmakeJob* CreateMatchBrowseMatchmakeJob();
    LanMatchJoinSessionJob* CreateMatchJoinSessionJob();
    LanMatchLeaveSessionJob* CreateMatchLeaveSessionJob();
    LanMatchDestroySessionJob* CreateMatchDestroySessionJob();
    LanMatchUpdateSessionSettingJob* CreateMatchUpdateSessionSettingJob();
    LanMatchJointSessionJob* CreateMatchJointSessionJob();
    void DeleteMatchCreateSessionJob(LanMatchCreateSessionJob*);
    void DeleteMatchRandomMatchmakeJob(LanMatchRandomMatchmakeJob*);
    void DeleteMatchBrowseMatchmakeJob(LanMatchBrowseMatchmakeJob*);
    void DeleteMatchJoinSessionJob(LanMatchJoinSessionJob*);
    void DeleteMatchLeaveSessionJob(LanMatchLeaveSessionJob*);
    void DeleteMatchDestroySessionJob(LanMatchDestroySessionJob*);
    void DeleteMatchUpdateSessionSettingJob(LanMatchUpdateSessionSettingJob*);
    void DestroyMatchJointSessionJob(LanMatchJointSessionJob*);
    uint32_t CreateSessionInfoList(uint16_t);
    LanMatchmakeSession* CreateMatchmakeSession();
    LanMatchMeshLayerController* CreateMatchMeshLayerController();
    uint64_t DeleteSessionInfoList(session::ISessionInfoList*);
    uint64_t DeleteMatchmakeSession(session::IMatchmakeSession*);
    uint64_t DeleteMatchMeshLayerController(session::MeshLayerController*);
    uint32_t GetMaxStationNum() const;
    uint32_t GetHeaderSize() const;
    uint32_t GetHostMigrationMode() const;
    uint32_t GetBrowseSessionInfoListSizeMax() const;
    uint32_t GetBrowseCommunityInfoListSizeMax() const;
    uint32_t GetMtuSizeMax() const;
    uint32_t GetMtuSizeMin() const;
    int GetDefaultRttInitialPulseInterval() const;
    bool IsMultiCandidatePermitted() const;
    bool IsJointSessionPermitted() const;
    bool IsMultiplayerPermitted() const;
};
}
}
}

#endif
