#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHMAKEUPDATEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHMAKEUPDATEJOB_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace lan {

class LanMatchmakeUpdateJob
{
public:
    LanMatchmakeUpdateJob(void* a1);
    ~LanMatchmakeUpdateJob();
    uint64_t Startup(uint32_t a1, uint32_t a2);
    uint32_t Cleanup();
    void NotifyDisconnectToMeshLayerController();
    uint64_t SendBrowseReplyMessage(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, int32_t a7, uint8_t a8);
    uint64_t SendBrowseMessage(uint32_t a1, uint32_t a2);
    uint32_t Update();
    uint32_t RequestBrowseMessage(uint8_t a1);
    bool IsBrowseMessageCompleted();
    void Trace(uint64_t a1);

private:
    unsigned char m_storage[1];
};

}}}

#endif
