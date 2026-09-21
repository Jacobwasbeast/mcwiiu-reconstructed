#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RELAY_STATION_PACKET_HANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RELAY_STATION_PACKET_HANDLER_HPP

#include "nn/pia.h"
#include "nn/pia/transport/StationPacketHandler.hpp"

namespace nn { namespace pia { namespace transport {

class RelayRouteManager;

class RelayStationPacketHandler : public StationPacketHandler
{
public:
    RelayStationPacketHandler();
    virtual ~RelayStationPacketHandler();
    virtual void Finalize();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;
    virtual void RelayMessage(const ProtocolMessageReader& reader);
    virtual unsigned int CheckMessage(const ProtocolMessageReader& reader);
    virtual unsigned int CheckReceive(const ProtocolMessageReader& reader);
    virtual Result Initialize(unsigned int stationNum, bool isStarConnection,
                              unsigned int maxProtocolNum);
    virtual ProtocolMessageWriter* AssignSingle(const ProtocolId& protocolId,
                                      ::nn::pia::StationIndex stationIndex,
                                      unsigned int size, bool isReliable,
                                      bool isPriority, bool isBroadcast);
    virtual ProtocolMessageWriter* AssignAll(const ProtocolId& protocolId, unsigned int size,
                                   bool isReliable, bool isPriority, bool isBroadcast);
    virtual ProtocolMessageWriter* AssignMulti(const ProtocolId& protocolId, unsigned int stationBitmap,
                             unsigned int size, bool isReliable,
                             bool isPriority, bool isBroadcast);

    Result Startup(PacketStream::Writer* writer, PacketStream::Reader* reader,
                   common::IPacketInput* packetInput, common::IPacketOutput* packetOutput,
                   unsigned long long* stationId,
                   const RelayRouteManager* relayRouteManager,
                   const common::CryptoSetting* cryptoSetting);

    const RelayRouteManager* m_relayRouteManager;
};

}}}

#endif
