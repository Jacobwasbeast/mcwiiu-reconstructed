#include "nn/pia/lan/LanNetworkFactory.hpp"
#include "nn/pia/lan/LanInputStream.hpp"
#include "nn/pia/lan/LanOutputStream.hpp"
#include "nn/pia/lan/LanConnectStationJob.hpp"
#include "nn/pia/lan/LanLeaveWithHostMigrationJob.hpp"
#include "nn/pia/lan/LanProcessHostMigrationJob.hpp"
#include "nn/pia/lan/LanMatchCreateSessionJob.hpp"
#include "nn/pia/lan/LanMatchRandomMatchmakeJob.hpp"
#include "nn/pia/lan/LanMatchBrowseMatchmakeJob.hpp"
#include "nn/pia/lan/LanMatchJoinSessionJob.hpp"
#include "nn/pia/lan/LanMatchLeaveSessionJob.hpp"
#include "nn/pia/lan/LanMatchDestroySessionJob.hpp"
#include "nn/pia/lan/LanMatchUpdateSessionSettingJob.hpp"
#include "nn/pia/lan/LanMatchJointSessionJob.hpp"
#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include "nn/pia/lan/LanMatchMeshLayerController.hpp"
#include "nn/pia/session/CreateMeshJob.hpp"
#include "nn/pia/session/JoinMeshJob.hpp"
#include "nn/pia/session/LeaveMeshJob.hpp"
#include "nn/pia/session/KickoutManageJob.hpp"
#include "nn/pia/session/lan.hpp"
#include "nn/pia/lan/LanSessionInfoPrivate.hpp"
#include "nn/pia/lan/LanProtocol.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Reconstruction.hpp"
#include "platform/wiiu/pead.hpp"
#include <cstring>
#include <new>

namespace {
template <typename T> T* makePia(unsigned int size)
{
    void* storage = pead::allocateFromSeadHeap(size, nn::pia::common::HeapManager::GetHeap());
    return storage == 0 ? 0 : new (storage) T();
}
template <typename T> void destroyPia(T* value)
{
    if (value != 0) {
        value->~T();
        pead::freeToSeadHeap(value);
    }
}
}

namespace nn { namespace pia { namespace lan {
LanNetworkFactory::LanNetworkFactory() : session::SessionNetworkFactory() {}
LanNetworkFactory::~LanNetworkFactory() {}
void LanNetworkFactory::CreateProtocols()
{
    transport::ProtocolManager* manager = transport::g_piaProtocolManager;
    if (manager == 0)
        return;
    transport::Protocol* storage = manager->AllocProtocol(0x1298);
    if (storage != 0) {
        LanProtocol* protocol = new (storage) LanProtocol();
        manager->CreateProtocolImpl(protocol, transport::ProtocolId(LanProtocol::cProtocolId));
    }
}
uint32_t LanNetworkFactory::CreateSelf() { return reinterpret_cast<uint32_t>(makePia<LanNetworkFactory>(4)); }
uint64_t LanNetworkFactory::DeleteSelf(transport::NetworkFactory* value) { destroyPia(value); return 0; }
LanInputStream* LanNetworkFactory::CreateInputStream() { return makePia<LanInputStream>(0x3c); }
LanOutputStream* LanNetworkFactory::CreateOutputStream() { return makePia<LanOutputStream>(0x214); }
void LanNetworkFactory::DeleteInputStream(LanInputStream* value) { destroyPia(reinterpret_cast<LanInputStream*>(reinterpret_cast<char*>(value) - 0xc)); }
void LanNetworkFactory::DeleteOutputStream(LanOutputStream* value) { destroyPia(reinterpret_cast<LanOutputStream*>(reinterpret_cast<char*>(value) - 0xc)); }
LanConnectStationJob* LanNetworkFactory::CreateConnectStationJob() { return makePia<LanConnectStationJob>(0x110); }
void LanNetworkFactory::DeleteConnectStationJob(transport::ConnectStationJob* value) { destroyPia(value); }
session::CreateMeshJob* LanNetworkFactory::CreateCreateMeshJob() { return makePia<session::CreateMeshJob>(0x48); }
session::JoinMeshJob* LanNetworkFactory::CreateJoinMeshJob() { return makePia<session::JoinMeshJob>(0xa8); }
session::LeaveMeshJob* LanNetworkFactory::CreateLeaveMeshJob() { return makePia<session::LeaveMeshJob>(0x58); }
uint32_t LanNetworkFactory::CreateProcessHostMigrationJob() { return reinterpret_cast<uint32_t>(makePia<LanProcessHostMigrationJob>(0x1d8)); }
session::KickoutManageJob* LanNetworkFactory::CreateKickoutManageJob() { return makePia<session::KickoutManageJob>(0x80); }
void LanNetworkFactory::DeleteCreateMeshJob(session::CreateMeshJob* value) { destroyPia(value); }
void LanNetworkFactory::DeleteJoinMeshJob(session::JoinMeshJob* value) { destroyPia(value); }
void LanNetworkFactory::DeleteLeaveMeshJob(session::LeaveMeshJob* value) { destroyPia(value); }
uint64_t LanNetworkFactory::DeleteProcessHostMigrationJob(common::StepSequenceJob* value) { destroyPia(value); return 0; }
void LanNetworkFactory::DeleteLeaveWithHostMigrationJob(LanLeaveWithHostMigrationJob* value) { destroyPia(value); }
void LanNetworkFactory::DeleteKickoutManageJob(session::KickoutManageJob* value) { destroyPia(value); }
LanLeaveWithHostMigrationJob* LanNetworkFactory::CreateLeaveWithHostMigrationJob() { return makePia<LanLeaveWithHostMigrationJob>(0x70); }
LanMatchCreateSessionJob* LanNetworkFactory::CreateMatchCreateSessionJob() { return makePia<LanMatchCreateSessionJob>(0x1d8); }
LanMatchRandomMatchmakeJob* LanNetworkFactory::CreateMatchRandomMatchmakeJob() { return makePia<LanMatchRandomMatchmakeJob>(0x80); }
LanMatchBrowseMatchmakeJob* LanNetworkFactory::CreateMatchBrowseMatchmakeJob() { return makePia<LanMatchBrowseMatchmakeJob>(0x168); }
LanMatchJoinSessionJob* LanNetworkFactory::CreateMatchJoinSessionJob() { return makePia<LanMatchJoinSessionJob>(0x70); }
LanMatchLeaveSessionJob* LanNetworkFactory::CreateMatchLeaveSessionJob() { return makePia<LanMatchLeaveSessionJob>(0x128); }
LanMatchDestroySessionJob* LanNetworkFactory::CreateMatchDestroySessionJob() { return makePia<LanMatchDestroySessionJob>(0x60); }
LanMatchUpdateSessionSettingJob* LanNetworkFactory::CreateMatchUpdateSessionSettingJob() { return makePia<LanMatchUpdateSessionSettingJob>(0x108); }
LanMatchJointSessionJob* LanNetworkFactory::CreateMatchJointSessionJob() { return makePia<LanMatchJointSessionJob>(0x70); }
void LanNetworkFactory::DeleteMatchCreateSessionJob(LanMatchCreateSessionJob* value) { destroyPia(value); }
void LanNetworkFactory::DeleteMatchRandomMatchmakeJob(LanMatchRandomMatchmakeJob* value) { destroyPia(value); }
void LanNetworkFactory::DeleteMatchBrowseMatchmakeJob(LanMatchBrowseMatchmakeJob* value) { destroyPia(value); }
void LanNetworkFactory::DeleteMatchJoinSessionJob(LanMatchJoinSessionJob* value) { destroyPia(value); }
void LanNetworkFactory::DeleteMatchLeaveSessionJob(LanMatchLeaveSessionJob* value) { destroyPia(value); }
void LanNetworkFactory::DeleteMatchDestroySessionJob(LanMatchDestroySessionJob* value) { destroyPia(value); }
void LanNetworkFactory::DeleteMatchUpdateSessionSettingJob(LanMatchUpdateSessionSettingJob* value) { destroyPia(value); }
void LanNetworkFactory::DestroyMatchJointSessionJob(LanMatchJointSessionJob* value) { destroyPia(value); }
LanMatchmakeSession* LanNetworkFactory::CreateMatchmakeSession() { return makePia<LanMatchmakeSession>(0x1b00); }
LanMatchMeshLayerController* LanNetworkFactory::CreateMatchMeshLayerController() { return makePia<LanMatchMeshLayerController>(0xf8); }
uint32_t LanNetworkFactory::CreateSessionInfoList(uint16_t requested)
{
    typedef session::SessionInfoList<LanSessionInfoPrivate> List;
    const uint16_t count = requested == 0 ? 1 : static_cast<uint16_t>(requested & 0xff);
    List* list = makePia<List>(0xc);
    if (list == 0)
        return 0;
    char* raw = reinterpret_cast<char*>(list);
    *reinterpret_cast<uint16_t*>(raw + 4) = 0;
    *reinterpret_cast<uint16_t*>(raw + 6) = count;
    LanSessionInfoPrivate** items = static_cast<LanSessionInfoPrivate**>(
        pead::allocateFromSeadHeap(static_cast<unsigned int>(count) * sizeof(void*), common::HeapManager::GetHeap()));
    *reinterpret_cast<LanSessionInfoPrivate***>(raw + 8) = items;
    if (items == 0)
        return reinterpret_cast<uint32_t>(list);
    std::memset(items, 0, static_cast<size_t>(count) * sizeof(void*));
    for (uint16_t i = 0; i < count; ++i)
        items[i] = makePia<LanSessionInfoPrivate>(0x758);
    return reinterpret_cast<uint32_t>(list);
}
uint64_t LanNetworkFactory::DeleteSessionInfoList(session::ISessionInfoList* value) { destroyPia(value); return 0; }
uint64_t LanNetworkFactory::DeleteMatchmakeSession(session::IMatchmakeSession* value) { destroyPia(value); return 0; }
uint64_t LanNetworkFactory::DeleteMatchMeshLayerController(session::MeshLayerController* value) { destroyPia(value); return 0; }
uint32_t LanNetworkFactory::GetMaxStationNum() const { return 0x10; }
uint32_t LanNetworkFactory::GetHeaderSize() const { return 0x1c; }
uint32_t LanNetworkFactory::GetHostMigrationMode() const { return 2; }
uint32_t LanNetworkFactory::GetBrowseSessionInfoListSizeMax() const { return 10; }
uint32_t LanNetworkFactory::GetBrowseCommunityInfoListSizeMax() const { return 0; }
uint32_t LanNetworkFactory::GetMtuSizeMax() const { return 0x554; }
uint32_t LanNetworkFactory::GetMtuSizeMin() const { return 0x554; }
int LanNetworkFactory::GetDefaultRttInitialPulseInterval() const { return 600; }
bool LanNetworkFactory::IsMultiCandidatePermitted() const { return true; }
bool LanNetworkFactory::IsJointSessionPermitted() const { return true; }
bool LanNetworkFactory::IsMultiplayerPermitted() const { return false; }
}}}
