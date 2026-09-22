#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHBROWSEMATCHMAKEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHBROWSEMATCHMAKEJOB_HPP

#include <cstdint>

namespace nn { namespace pia { namespace lan {

class LanMatchBrowseMatchmakeJob
{
public:
    LanMatchBrowseMatchmakeJob();
    ~LanMatchBrowseMatchmakeJob();
    void Cleanup();
    uint32_t BrowseMatchmake();
    uint32_t WaitBrowseMatchmake();
    uint32_t WaitRequestSessionInfo();
    void Trace(uint64_t a1);
    uint64_t StartupImpl(uint32_t a1, uint32_t a2, uint32_t a3);
    void StartupImpl(const unsigned char* a1, uint32_t a2);

private:
    unsigned char m_storage[96];
};

}}}

#endif
