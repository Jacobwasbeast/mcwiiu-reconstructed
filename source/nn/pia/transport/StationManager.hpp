#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATIONMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATIONMANAGER_HPP

#include "nn/pia/Reconstruction.hpp"

namespace nn { namespace pia { namespace transport {

class Station;

class StationManager
{
public:
    enum { cStationNumMax = 32 };

    void* GetStation(const common::StationAddress& address);

private:
    unsigned char m_pad00[0x28];
    unsigned int m_stationNum;
    Station* m_stations[cStationNumMax];
};

}}}

#endif
