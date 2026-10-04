#include "nn/pia/lan/LanNetworkFactory.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/IPacketInput.hpp"
#include "nn/pia/common/IPacketOutput.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/lan/LanConnectStationJob.hpp"
#include "nn/pia/lan/LanInputStream.hpp"
#include "nn/pia/lan/LanLeaveWithHostMigrationJob.hpp"
#include "nn/pia/lan/LanMatchDestroySessionJob.hpp"
#include "nn/pia/lan/LanMatchJointSessionJob.hpp"
#include "nn/pia/lan/LanMatchLeaveSessionJob.hpp"
#include "nn/pia/lan/LanMatchMeshLayerController.hpp"
#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include "nn/pia/lan/LanOutputStream.hpp"
#include "nn/pia/lan/LanProtocol.hpp"
#include "nn/pia/lan/LanSessionInfoPrivate.hpp"
#include "nn/pia/session/CreateMeshJob.hpp"
#include "nn/pia/session/JoinMeshJob.hpp"
#include "nn/pia/session/KickoutManageJob.hpp"
#include "nn/pia/session/LeaveMeshJob.hpp"
#include "nn/pia/session/lan.hpp"
#include <string.h>

namespace nn { namespace pia { namespace lan {

LanNetworkFactory::LanNetworkFactory()
{
}

LanNetworkFactory::~LanNetworkFactory()
{
}

unsigned int LanNetworkFactory::GetMaxStationNum() const
{
    return 0x10;
}

unsigned int LanNetworkFactory::GetHeaderSize() const
{
    return 0x1c;
}

unsigned int LanNetworkFactory::GetMtuSizeMax() const
{
    return 0x554;
}

unsigned int LanNetworkFactory::GetMtuSizeMin() const
{
    return 0x554;
}

int LanNetworkFactory::GetDefaultRttInitialPulseInterval() const
{
    return 600;
}

void LanNetworkFactory::DeleteSelf(transport::NetworkFactory* factory)
{
    transport::DestroyObject(factory);
}

Result LanNetworkFactory::CreateProtocols()
{
    transport::Transport* transport = transport::g_piaTransport;
    if (transport == 0)
        return static_cast<Result>(0x10c08);
    if (!transport->m_protocolManager.m_started)
    {
        transport::UnreliableProtocol* storage =
            transport->m_protocolManager.AllocProtocol(sizeof(LanProtocol));
        LanProtocol* protocol = storage != 0
            ? new (static_cast<void*>(storage)) LanProtocol() : 0;
        transport->m_protocolManager.CreateProtocolImpl(
            protocol, transport::ProtocolId(LanProtocol::cProtocolId));
    }
    return 0;
}

common::IPacketInput* LanNetworkFactory::CreateInputStream()
{
    return transport::CreateObject<LanInputStream>();
}

common::IPacketOutput* LanNetworkFactory::CreateOutputStream()
{
    return transport::CreateObject<LanOutputStream>();
}

void LanNetworkFactory::DeleteInputStream(common::IPacketInput* stream)
{
    if (stream != 0)
        transport::DestroyObject(static_cast<common::SocketStreamBase*>(
            static_cast<LanInputStream*>(stream)));
}

void LanNetworkFactory::DeleteOutputStream(common::IPacketOutput* stream)
{
    if (stream != 0)
        transport::DestroyObject(static_cast<common::SocketStreamBase*>(
            static_cast<LanOutputStream*>(stream)));
}

transport::ConnectStationJob* LanNetworkFactory::CreateConnectStationJob()
{
    return transport::CreateObject<LanConnectStationJob>();
}

void LanNetworkFactory::DeleteConnectStationJob(transport::ConnectStationJob* job)
{
    transport::DestroyObject(job);
}

unsigned int LanNetworkFactory::GetHostMigrationMode() const
{
    return 2;
}

unsigned int LanNetworkFactory::GetBrowseSessionInfoListSizeMax() const
{
    return 10;
}

unsigned int LanNetworkFactory::GetBrowseCommunityInfoListSizeMax() const
{
    return 0;
}

bool LanNetworkFactory::IsMultiCandidatePermitted() const
{
    return true;
}

bool LanNetworkFactory::IsJointSessionPermitted() const
{
    return true;
}

bool LanNetworkFactory::IsMultiplayerPermitted() const
{
    return false;
}

common::StepSequenceJob* LanNetworkFactory::CreateCreateMeshJob()
{
    return transport::CreateObject<session::CreateMeshJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateJoinMeshJob()
{
    return transport::CreateObject<session::JoinMeshJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateLeaveMeshJob()
{
    return transport::CreateObject<session::LeaveMeshJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateLeaveWithHostMigrationJob()
{
    return transport::CreateObject<LanLeaveWithHostMigrationJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateKickoutManageJob()
{
    return transport::CreateObject<session::KickoutManageJob>();
}

void LanNetworkFactory::DeleteCreateMeshJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteJoinMeshJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteLeaveMeshJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteProcessHostMigrationJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteLeaveWithHostMigrationJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteKickoutManageJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

common::StepSequenceJob* LanNetworkFactory::CreateMatchDestroySessionJob()
{
    return transport::CreateObject<LanMatchDestroySessionJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateMatchJointSessionJob()
{
    return transport::CreateObject<LanMatchJointSessionJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateMatchLeaveSessionJob()
{
    return transport::CreateObject<LanMatchLeaveSessionJob>();
}

void LanNetworkFactory::DeleteMatchCreateSessionJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteMatchRandomMatchmakeJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteMatchBrowseMatchmakeJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteMatchJoinSessionJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteMatchLeaveSessionJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteMatchDestroySessionJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DestroyMatchJointSessionJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteMatchUpdateSessionSettingJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

session::ISessionInfoList* LanNetworkFactory::CreateSessionInfoList(unsigned short size)
{
    typedef session::SessionInfoList<LanSessionInfoPrivate> List;
    unsigned int count = 1;
    if (size != 0)
        count = size & 0xff;
    List* list = transport::CreateObject<List>();
    if (list == 0)
        return 0;
    list->m_maxSize = static_cast<unsigned short>(count);
    list->m_size = 0;
    session::ISessionInfo** items = static_cast<session::ISessionInfo**>(
        pead::allocateFromSeadHeap(count * sizeof(session::ISessionInfo*),
                                   common::HeapManager::GetHeap()));
    if (items != 0)
    {
        for (unsigned int index = 0; index < count; ++index)
            items[index] = 0;
    }
    list->m_items = items;
    for (unsigned int index = 0; index < list->m_maxSize; ++index)
    {
        void* storage = pead::allocateFromSeadHeap(sizeof(LanSessionInfoPrivate),
                                                   common::HeapManager::GetHeap());
        LanSessionInfoPrivate* info = 0;
        if (storage != 0)
        {
            memset(storage, 0, sizeof(LanSessionInfoPrivate));
            info = new (storage) LanSessionInfoPrivate();
        }
        list->m_items[index] = info;
    }
    return list;
}

session::IMatchmakeSession* LanNetworkFactory::CreateMatchmakeSession()
{
    return transport::CreateObject<LanMatchmakeSession>();
}

session::MeshLayerController* LanNetworkFactory::CreateMatchMeshLayerController()
{
    return transport::CreateObject<LanMatchMeshLayerController>();
}

void LanNetworkFactory::DeleteSessionInfoList(session::ISessionInfoList* list)
{
    transport::DestroyObject(list);
}

void LanNetworkFactory::DeleteMatchmakeSession(session::IMatchmakeSession* session)
{
    transport::DestroyObject(session);
}

void LanNetworkFactory::DeleteMatchMeshLayerController(session::MeshLayerController* controller)
{
    transport::DestroyObject(controller);
}

}}}
