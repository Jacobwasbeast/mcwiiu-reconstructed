#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PACKET_HANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PACKET_HANDLER_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/CryptoSetting.hpp"
#include "nn/pia/transport/PacketStream.hpp"

namespace nn { namespace pia { namespace common {
class Packet;
class StationAddress;
class Time;
class IPacketInput;
class IPacketOutput;
}}}

namespace nn { namespace pia { namespace transport {

class ProtocolMessageReader;

class PacketHandler
{
public:
    PacketHandler();
    virtual ~PacketHandler();
    virtual void Finalize();
    virtual void Cleanup();
    virtual void BeginDispatch(const common::Time& time);
    virtual void EndDispatch(const common::Time& time);
    virtual unsigned int GenerateConnectionId() const;
    virtual void Trace(unsigned long long value) const;
    virtual void RelayMessage(const ProtocolMessageReader& reader);
    virtual void BeginIteration();
    virtual bool IsEndIteration() const;
    virtual void NextIteration();
    virtual unsigned int CheckPacket(const common::Packet& packet);
    virtual unsigned int CheckMessage(const ProtocolMessageReader& reader);
    virtual unsigned int CheckReceive(const ProtocolMessageReader& reader);
    virtual bool CheckFiltering(const common::StationAddress& stationAddress) const;
    virtual unsigned int IsBroadcastSupported() const;

    Result Startup(PacketStream::Writer* writer, PacketStream::Reader* reader,
                   common::IPacketInput* packetInput, common::IPacketOutput* packetOutput,
                   unsigned long long* stationId,
                   const common::CryptoSetting* cryptoSetting);
    Result UpdateSetting(const common::CryptoSetting* cryptoSetting);
};

}}}

#endif
