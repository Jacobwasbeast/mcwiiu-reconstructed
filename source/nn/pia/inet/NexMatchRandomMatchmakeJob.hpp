#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHRANDOMMATCHMAKEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHRANDOMMATCHMAKEJOB_HPP

#include <stdint.h>

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace session { class JoinRandomSessionSetting; }}}

namespace nn { namespace pia { namespace inet {

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
    uint64_t NoticeOwnerChangeEvent(uint32_t a1, uint64_t a2);
    void NoticeUnregisterEvent(uint32_t a1);
    void Trace(uint64_t a1) const;
    uint64_t StartupImpl(const session::JoinRandomSessionSetting* a1);
    void CleanupImpl();
    void SetNextStepAtWaitCreateMesh();
    void SetNextStepAtWaitJoinMesh();
    uint32_t SetNextStepAtFailedCreateMesh(Result a1);
    uint32_t SetNextStepAtFailedJoinMesh(Result a1);
    uint32_t SetNextStepAtWaitLeaveMesh();

private:
    uint64_t state_;
};

}}}

#endif
