#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHJOINSESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHJOINSESSIONJOB_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace lan {

class LanMatchJoinSessionJob
{
public:
    LanMatchJoinSessionJob();
    ~LanMatchJoinSessionJob();
    uint32_t JoinMatchmakeSession();
    uint32_t WaitJoinMatchmake();
    uint32_t GetStationConnectionInfo();
    uint32_t WaitGetStationConnectionInfo();
    uint32_t LeaveMatchmakeSession();
    uint32_t WaitLeaveMatchmakeSession();
    uint32_t WaitForRetryJoinMatchmakeSession();
    uint32_t WaitRequestSessionInfo();
    void Cleanup();
    uint64_t NoticeOwnerChangeEvent(int32_t a1, uint32_t a2, uint32_t a3);
    void Trace(uint64_t a1);
    void SetNextStepAtWaitJoinMesh();
    uint32_t SetNextStepAtFailedJoinMesh(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);
    uint32_t SetNextStepAtWaitLeaveMesh();
    uint32_t SetNextStepAtMeshCleanup();
    uint64_t StartupImpl(uint32_t a1, int32_t a2);

private:
    unsigned char m_storage[1];
};

}}}

#endif
