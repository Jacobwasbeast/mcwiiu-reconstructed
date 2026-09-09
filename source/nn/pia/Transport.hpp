#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_HPP
#include <cstdint>
#include "nn/pia.h"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/common/CryptoSetting.hpp"
namespace nn { namespace pia { namespace transport {
class Transport {
public:
    Transport();
    Result Startup(unsigned int payloadSize, const common::CryptoSetting* cryptoSetting);
    Result UpdateSetting(const common::CryptoSetting* cryptoSetting);
    Result ConvertToStationIndex(StationIndex*, std::uint64_t) const;
    Result m_transportStatus;
    ProtocolManager m_protocolManager;
    StationIdTable* m_stationIdTable;
};
extern Transport* g_piaTransport;
StationIndex Conv2StationIndex(std::uint64_t);
}}}
#endif
