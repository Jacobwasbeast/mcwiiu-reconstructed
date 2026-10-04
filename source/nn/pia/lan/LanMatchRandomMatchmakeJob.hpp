#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHRANDOMMATCHMAKEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHRANDOMMATCHMAKEJOB_HPP

#include <stdint.h>
#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace session { class JoinRandomSessionSetting; }}}

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
    void NoticeOwnerChangeEvent(uint32_t ownerId, uint64_t principalId);
    void Cleanup();
    void Trace(uint64_t value) const;
    Result StartupImpl(const session::JoinRandomSessionSetting* setting);
    void SetNextStepAtWaitCreateMesh();
    void SetNextStepAtWaitJoinMesh();
    uint32_t SetNextStepAtFailedCreateMesh(Result result);
    uint32_t SetNextStepAtFailedJoinMesh(Result result);
    uint32_t SetNextStepAtWaitLeaveMesh();

private:
    unsigned char m_storage[1];
};

}}}

#endif
