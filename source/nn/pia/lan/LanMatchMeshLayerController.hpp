#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHMESHLAYERCONTROLLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHMESHLAYERCONTROLLER_HPP

#include <stdint.h>

namespace nn { namespace pia { namespace lan {

class LanMatchMeshLayerController
{
public:
    LanMatchMeshLayerController();
    ~LanMatchMeshLayerController();
    uint64_t Startup(uint32_t a1, int32_t a2);
    void MeshCleanup();
    void Cleanup();
    void MeshStartup(void* a1, bool a2);
    uint64_t CheckConnectedStatus();
    void Trace(uint64_t a1);
    void GetMatchmakeSessionTotalStations();
    int16_t GetMatchmakeSessionTotalParticipants();
    uint32_t IsParticipated(uint32_t a1, int32_t a2, int32_t a3);
    void SetConnectionStatusDisconnected();

private:
    unsigned char m_storage[1];
};

}}}

#endif
