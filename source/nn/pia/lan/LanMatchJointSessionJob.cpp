#include "nn/pia/lan/LanMatchJointSessionJob.hpp"
#include <cstring>

namespace nn { namespace pia { namespace lan {

LanMatchJointSessionJob::LanMatchJointSessionJob()
{
    std::memset(m_storage, 0, sizeof(m_storage));
}

LanMatchJointSessionJob::~LanMatchJointSessionJob()
{
}

uint32_t LanMatchJointSessionJob::CallSessionEvent()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::SendInvitationAsCompanion()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::WaitForAnswerToInvitation()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::StartRandomMatchmake()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::WaitRandomMatchmake()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::StartJoinMatchmakeSession()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::WaitJoinMatchmakeSession()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::SendNextSessionId()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::WaitCompanionStationPrepared()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::SendAnswerToInvitation()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::WaitNextSessionId()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::StartUpdateSessionKey()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::WaitUpdateSessionKey()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::SendPreparedForMigrateSession()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::WaitUntilLeaderLeavesMesh()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::StartLeavePreviousMesh()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::WaitLeavePreviousMesh()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::StartLeavePreviousMatchmakeSession()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::WaitLeavePreviousMatchmakeSession()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::MeshRestart()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::StartCreateNextMesh()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::WaitCreateNextMesh()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::StartGetNextMeshHostStationConnectionInfo()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::WaitGetNextMeshHostStationConnectionInfo()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::StartJoinNextMesh()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::WaitJoinNextMesh()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::WaitCompanionStation()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::WaitHostStationId()
{
    return 0;
}

uint32_t LanMatchJointSessionJob::ProcessComplete()
{
    return 0;
}

void LanMatchJointSessionJob::Trace(uint64_t a1)
{
}

uint64_t LanMatchJointSessionJob::StartupJoinRandomJointSessionImpl(void* a1)
{
    return 0;
}

uint64_t LanMatchJointSessionJob::StartupCreateJointSessionImpl(void* a1)
{
    return 0;
}

uint64_t LanMatchJointSessionJob::StartupJoinJointSessionImpl(void* a1)
{
    return 0;
}

uint64_t LanMatchJointSessionJob::StartupLeaveJointSessionImpl()
{
    return 0;
}

uint64_t LanMatchJointSessionJob::StartupDestroyJointSessionImpl()
{
    return 0;
}

uint64_t LanMatchJointSessionJob::StartupPassiveProcessImpl(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, int32_t a5, int32_t a6, int32_t a7)
{
    return 0;
}

uint64_t LanMatchJointSessionJob::ReceiveLeaveInvitationImpl(uint32_t a1, void* a2, uint16_t a3, uint64_t a4)
{
    return 0;
}

uint32_t LanMatchJointSessionJob::GetJointSessionStateForDebug()
{
    uint32_t value = 0;
    std::memcpy(&value, m_storage + 0x1b4, sizeof(value));
    return value;
}

uint64_t LanMatchJointSessionJob::ReceiveNextSessionIdImpl(uint32_t a1, uint32_t a2, void* a3, uint8_t a4, uint64_t a5)
{
    return 0;
}

void LanMatchJointSessionJob::ReceiveLeaveMeshInvitationImpl(uint32_t a1, uint64_t a2)
{
}

void LanMatchJointSessionJob::ReceiveCompletionResponseImpl(uint32_t a1, uint64_t a2)
{
}

void LanMatchJointSessionJob::CleanupImpl()
{
}

}}}
