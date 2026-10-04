#include "nn/pia/transport/RelayStationPacketHandler.hpp"

namespace nn { namespace pia { namespace transport {

bool RelayStationPacketHandler::checkDerivedRuntimeTypeInfo(const pead::RuntimeTypeInfo::Interface* type) const
{
    return checkDerivedRuntimeTypeInfoStatic(type);
}

const pead::RuntimeTypeInfo::Interface* RelayStationPacketHandler::getRuntimeTypeInfo() const
{
    return getRuntimeTypeInfoStatic();
}

Result RelayStationPacketHandler::Startup(PacketStream::Writer* writer,
                                          PacketStream::Reader* reader,
                                          common::IPacketInput* packetInput,
                                          common::IPacketOutput* packetOutput,
                                          unsigned long long* stationId,
                                          const RelayRouteManager* relayRouteManager,
                                          const common::CryptoSetting* cryptoSetting)
{
    const Result result = StationPacketHandler::Startup(writer, reader, packetInput,
                                                        packetOutput, stationId,
                                                        cryptoSetting);
    if (result != 0)
        return result;

    m_relayRouteManager = relayRouteManager;
    return 0;
}

}}}
