#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHMESHLAYERCONTROLLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHMESHLAYERCONTROLLER_HPP

#include <stdint.h>

#include "nn/pia/Common.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn { namespace pia { namespace session { class IMatchmakeSession; }}}

namespace nn { namespace pia { namespace lan {

class LanMatchMeshLayerController
{
public:
    LanMatchMeshLayerController();
    ~LanMatchMeshLayerController();
    Result Startup(const session::Session::StartupSetting& setting);
    void MeshCleanup();
    void Cleanup();
    void MeshStartup(session::IMatchmakeSession* matchmakeSession, bool isHost);
    uint64_t CheckConnectedStatus();
    void Trace(unsigned long long value) const;
    void GetMatchmakeSessionTotalStations() const;
    int16_t GetMatchmakeSessionTotalParticipants() const;
    bool IsParticipated(unsigned int index, unsigned long long stationId) const;
    void SetConnectionStatusDisconnected();

private:
    unsigned char m_storage[1];
};

}}}

#endif
