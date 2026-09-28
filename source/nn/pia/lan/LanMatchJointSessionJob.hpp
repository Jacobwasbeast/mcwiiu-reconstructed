#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHJOINTSESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHJOINTSESSIONJOB_HPP

#include <stdint.h>

namespace nn
{
namespace pia
{
namespace lan
{

class LanMatchJointSessionJob
{
public:
    LanMatchJointSessionJob();
    ~LanMatchJointSessionJob();
    uint32_t CallSessionEvent();
    uint32_t SendInvitationAsCompanion();
    uint32_t WaitForAnswerToInvitation();
    uint32_t StartRandomMatchmake();
    uint32_t WaitRandomMatchmake();
    uint32_t StartJoinMatchmakeSession();
    uint32_t WaitJoinMatchmakeSession();
    uint32_t SendNextSessionId();
    uint32_t WaitCompanionStationPrepared();
    uint32_t SendAnswerToInvitation();
    uint32_t WaitNextSessionId();
    uint32_t StartUpdateSessionKey();
    uint32_t WaitUpdateSessionKey();
    uint32_t SendPreparedForMigrateSession();
    uint32_t WaitUntilLeaderLeavesMesh();
    uint32_t StartLeavePreviousMesh();
    uint32_t WaitLeavePreviousMesh();
    uint32_t StartLeavePreviousMatchmakeSession();
    uint32_t WaitLeavePreviousMatchmakeSession();
    uint32_t MeshRestart();
    uint32_t StartCreateNextMesh();
    uint32_t WaitCreateNextMesh();
    uint32_t StartGetNextMeshHostStationConnectionInfo();
    uint32_t WaitGetNextMeshHostStationConnectionInfo();
    uint32_t StartJoinNextMesh();
    uint32_t WaitJoinNextMesh();
    uint32_t WaitCompanionStation();
    uint32_t WaitHostStationId();
    uint32_t ProcessComplete();
    void Trace(uint64_t a1);
    uint64_t StartupJoinRandomJointSessionImpl(void* a1);
    uint64_t StartupCreateJointSessionImpl(void* a1);
    uint64_t StartupJoinJointSessionImpl(void* a1);
    uint64_t StartupLeaveJointSessionImpl();
    uint64_t StartupDestroyJointSessionImpl();
    uint64_t StartupPassiveProcessImpl(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, int32_t a5,
                                       int32_t a6, int32_t a7);
    uint64_t ReceiveLeaveInvitationImpl(uint32_t a1, void* a2, uint16_t a3, uint64_t a4);
    uint32_t GetJointSessionStateForDebug();
    uint64_t ReceiveNextSessionIdImpl(uint32_t a1, uint32_t a2, void* a3, uint8_t a4, uint64_t a5);
    void ReceiveLeaveMeshInvitationImpl(uint32_t a1, uint64_t a2);
    void ReceiveCompletionResponseImpl(uint32_t a1, uint64_t a2);
    void CleanupImpl();

private:
    unsigned char m_storage[328];
};

}
}
}

#endif
