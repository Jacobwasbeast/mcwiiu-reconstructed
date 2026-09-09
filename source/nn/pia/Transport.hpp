#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_HPP
#include <cstdint>
#include "nn/pia.h"
#include "nn/pia/StationIdTable.hpp"
namespace nn { namespace pia { namespace transport {
class Transport {
public:
    Transport();
    Result ConvertToStationIndex(StationIndex*, std::uint64_t) const;
    Result m_transportStatus;
    StationIdTable* m_stationIdTable;
};
extern Transport* g_piaTransport;
StationIndex Conv2StationIndex(std::uint64_t);
}}}
#endif
