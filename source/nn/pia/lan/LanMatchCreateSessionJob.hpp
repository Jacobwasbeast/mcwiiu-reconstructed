#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHCREATESESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHCREATESESSIONJOB_HPP

#include <cstdint>

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
    void Trace(uint64_t a1);
    uint64_t StartupImpl(uint32_t a1, uint32_t a2);
    void SetNextStepAtWaitCreateMesh();
    uint32_t SetNextStepAtFailedCreateMesh(uint32_t a1, uint32_t a2, uint32_t a3);

private:
    unsigned char m_storage[112];
};

}
}
}

#endif
