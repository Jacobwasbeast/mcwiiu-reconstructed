#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATIONMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATIONMANAGER_HPP

#include "nn/pia/Reconstruction.hpp"

namespace nn { namespace pia { namespace transport {

class StationManager
{
public:
    void* GetStation(const common::StationAddress& address);
};

}}}

#endif
