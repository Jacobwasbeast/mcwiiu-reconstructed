#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHCREATESESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHCREATESESSIONJOB_HPP

#include <stdint.h>
#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace session { class CreateSessionSetting; }}}

namespace nn
{
namespace pia
{
namespace lan
{

class LanMatchCreateSessionJob
{
public:
    LanMatchCreateSessionJob();
    ~LanMatchCreateSessionJob();
    uint32_t CreateMatchmakeSession();
    uint32_t WaitCreateMatchmake();
    uint32_t OpenParticipate();
    uint32_t WaitOpenParticipate();
    uint32_t UnregisterGathering();
    uint32_t WaitUnregisterGathering();
    void Cleanup();
    void Trace(uint64_t a1) const;
    Result StartupImpl(const session::CreateSessionSetting* setting);
    void SetNextStepAtWaitCreateMesh();
    uint32_t SetNextStepAtFailedCreateMesh(Result result);

private:
    unsigned char m_storage[112];
};

}
}
}

#endif
