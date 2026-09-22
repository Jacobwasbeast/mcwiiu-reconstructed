#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHRANDOMMATCHMAKEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHRANDOMMATCHMAKEJOB_HPP

#include <cstdint>

namespace nn { namespace pia { namespace lan {

class LanMatchRandomMatchmakeJob
{
public:
    LanMatchRandomMatchmakeJob();
    ~LanMatchRandomMatchmakeJob();
    uint32_t RandomMatchmake();
    uint32_t WaitRandomMatchmake();
    uint32_t UnregisterGathering();
    uint32_t WaitUnregisterGathering();
    uint32_t LeaveMatchmakeSession();
    uint32_t WaitLeaveMatchmakeSession();
    uint32_t WaitForRetryRandomMatchmake();
    uint32_t WaitRequestSessionInfo();
    uint64_t NoticeOwnerChangeEvent(int32_t a1, uint32_t a2, uint32_t a3);
    void Cleanup();
    void Trace(uint64_t a1);
    uint64_t StartupImpl(uint32_t a1, uint32_t a2);
    void SetNextStepAtWaitCreateMesh();
    void SetNextStepAtWaitJoinMesh();
    uint32_t SetNextStepAtFailedCreateMesh(void* a1);
    uint32_t SetNextStepAtFailedJoinMesh(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4);
    uint32_t SetNextStepAtWaitLeaveMesh();

private:
    unsigned char m_storage[1];
};

}}}

#endif
