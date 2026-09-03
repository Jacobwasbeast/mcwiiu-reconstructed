#include "vendor/__N_28_transport_StationManager_cpp_626c851d.hpp"

uint32_t __N_28_transport_StationManager_cpp_626c851d::isValidStationIndex(uint32_t stationIndex)
{
    return stationIndex <= 0x1f || stationIndex == 0xfe;
}
