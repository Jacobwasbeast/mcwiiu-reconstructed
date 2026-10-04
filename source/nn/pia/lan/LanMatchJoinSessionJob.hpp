#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHJOINSESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHJOINSESSIONJOB_HPP

#include <stdint.h>

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace session { class JoinSessionSetting; }}}

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
    void NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId);
    void Trace(unsigned long long value) const;
    void SetNextStepAtWaitJoinMesh();
    uint32_t SetNextStepAtFailedJoinMesh(Result result);
    uint32_t SetNextStepAtWaitLeaveMesh();
    uint32_t SetNextStepAtMeshCleanup();
    Result StartupImpl(const session::JoinSessionSetting* setting);

private:
    unsigned char m_storage[1];
};

}}}

#endif
