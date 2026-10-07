#include "vendor/__N_28_transport_StationManager_cpp_626c851d.hpp"

unsigned int __N_28_transport_StationManager_cpp_626c851d::isValidStationIndex(nn::pia::StationIndex stationIndex)
{
    if (stationIndex <= 0x1f || stationIndex == 0xfe)
        return 1;
    return 0;
}
