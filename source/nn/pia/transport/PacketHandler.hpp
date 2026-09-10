#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PACKET_HANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PACKET_HANDLER_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/common/CryptoSetting.hpp"
#include "nn/pia/transport/PacketStream.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/transport/ProtocolMessageWriter.hpp"

namespace nn { namespace pia { namespace common {
class Packet;
class Time;
class IPacketInput;
class IPacketOutput;
}}}

namespace nn { namespace pia { namespace transport {

class PacketAnalyzer;

class PacketHandler
{
public:
    enum { cPacketHeaderSize = 36, cPayloadSizeMax = 1436 };

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

    PacketStream::Writer* m_writer;
    PacketStream::Reader* m_reader;
    ProtocolMessageWriter m_protocolMessageWriter;
    ProtocolMessageReader m_protocolMessageReader;
    unsigned int m_payloadSize;
    unsigned int m_packetSize;
    unsigned int m_field358;
    unsigned short m_field35c;
    unsigned short m_field35e;
    unsigned int m_field360;
    unsigned int m_field364;
    PacketHandler* m_self;
    PacketAnalyzer* m_sendPacketAnalyzer;
    PacketAnalyzer* m_sendUnicastPacketAnalyzer;
    PacketAnalyzer* m_sendBroadcastPacketAnalyzer;
    PacketAnalyzer* m_receivePacketAnalyzer;
    common::CryptoSetting m_cryptoSetting;
    unsigned long long* m_stationId;
    common::IPacketInput* m_packetInput;
    common::IPacketOutput* m_packetOutput;
};

}}}

#endif
