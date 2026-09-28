#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHRANDOMMATCHMAKEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHRANDOMMATCHMAKEJOB_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace inet {
struct nn_pia_Result;

class NexMatchRandomMatchmakeJob
{
public:
    NexMatchRandomMatchmakeJob();
    ~NexMatchRandomMatchmakeJob();
    uint32_t LeaveJoinedMatchmakeSession();
    uint32_t WaitLeaveJoinedMatchmakeSession();
    uint32_t RandomMatchmake();
    uint32_t WaitRandomMatchmake();
    uint32_t WaitNotification();
    uint32_t StartNatSession();
    uint32_t WaitStartNatSession();
    uint32_t LeaveMatchmakeSession();
    uint32_t WaitLeaveCurrentMatchmakeSession();
    uint32_t WaitLeaveBufferMatchmakeSession();
    uint32_t ChangeParticipation();
    uint32_t WaitChangeParticipation();
    uint32_t CleanupForRetryJoinMesh();
    uint32_t WaitForRetryJoinMesh();
    uint64_t NoticeOwnerChangeEvent(int32_t a1, uint32_t a2, uint32_t a3);
    void NoticeUnregisterEvent(uint32_t a1);
    void Trace(uint64_t a1);
    uint64_t StartupImpl(uint32_t a1, uint32_t a2);
    void CleanupImpl();
    void SetNextStepAtWaitCreateMesh();
    void SetNextStepAtWaitJoinMesh();
    uint32_t SetNextStepAtFailedCreateMesh(uint32_t a1, uint32_t a2, uint32_t a3);
    uint32_t SetNextStepAtFailedJoinMesh(nn_pia_Result* a1);
    uint32_t SetNextStepAtWaitLeaveMesh();

private:
    uint64_t state_;
};

}}}

#endif
