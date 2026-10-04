#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHDESTROYSESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHDESTROYSESSIONJOB_HPP

#include "nn/pia/session/DestroySessionJob.hpp"

namespace nn { namespace pia { namespace lan {

class LanMatchDestroySessionJob : public session::DestroySessionJob
{
public:
    LanMatchDestroySessionJob();
    ~LanMatchDestroySessionJob();
    uint32_t UnregisterCurrentMatchmakeSession();
    uint32_t WaitUnregisterCurrentMatchmakeSession();
    void Cleanup();
    void Trace(unsigned long long value) const;
    Result StartupImpl();
    void SetNextStepAtWaitDestroyMesh();
    void SetNextStepAtMeshCleanup();

};

}}}

#endif
