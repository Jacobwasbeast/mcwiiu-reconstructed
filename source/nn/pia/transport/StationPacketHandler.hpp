#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_PACKET_HANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_PACKET_HANDLER_HPP

#include "nn/pia.h"
#include "nn/pia/transport/PacketHandler.hpp"

namespace nn { namespace pia { namespace transport {

class StationConnectionInfoTable;

class StationPacketHandler : public PacketHandler
{
public:
    StationPacketHandler();
    virtual ~StationPacketHandler();
    virtual void Finalize();
    virtual void Cleanup();
    virtual void BeginDispatch(const common::Time& time);
    virtual void EndDispatch(const common::Time& time);
    virtual unsigned int GenerateConnectionId() const;
    virtual void Trace(unsigned long long value) const;
    virtual unsigned int CheckPacket(const common::Packet& packet);
    virtual unsigned int CheckMessage(const ProtocolMessageReader& reader);
    virtual unsigned int CheckReceive(const ProtocolMessageReader& reader);
    virtual unsigned int IsBroadcastSupported() const;
    virtual Result Initialize(unsigned int stationNum, bool isStarConnection,
                              unsigned int maxProtocolNum);
    virtual unsigned int AssignSingle(const ProtocolId& protocolId,
                                      ::nn::pia::StationIndex stationIndex,
                                      unsigned int size, bool isReliable,
                                      bool isPriority, bool isBroadcast);
    virtual unsigned int AssignAll(const ProtocolId& protocolId, unsigned int size,
                                   bool isReliable, bool isPriority, bool isBroadcast);
    virtual void AssignMulti(const ProtocolId& protocolId, unsigned int size,
                             unsigned int stationBitmap, bool isReliable,
                             bool isPriority, bool isBroadcast);
    void AssignMulti(const ProtocolId& protocolId, unsigned int size,
                     unsigned int stationBitmap, bool isReliable, bool isPriority,
                     unsigned long long destinationStationIds, bool isBroadcast);

    Result Startup(PacketStream::Writer* writer, PacketStream::Reader* reader,
                   common::IPacketInput* packetInput, common::IPacketOutput* packetOutput,
                   unsigned long long* stationId,
                   const common::CryptoSetting* cryptoSetting);

    static StationConnectionInfoTable* GetStationConnectionInfoTable();
};

}}}

#endif
