#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PACKET_STREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_PACKET_STREAM_HPP

#include "nn/pia/Reconstruction.hpp"

namespace nn { namespace pia { namespace common {
class Packet;
}}}

namespace nn { namespace pia { namespace transport {

class PacketStream
{
public:
    class Writer
    {
    public:
        Writer(PacketStream* packetStream)
            : m_packetStream(packetStream), m_field04(-1), m_field08(-1), m_field0c(0)
        {
        }

        PacketStream* m_packetStream;
        int m_field04;
        int m_field08;
        unsigned int m_field0c;
    };

    class Reader
    {
    public:
        Reader(PacketStream* packetStream)
            : m_packetStream(packetStream), m_field04(-1), m_field08(-1), m_field0c(0)
        {
        }

        PacketStream* m_packetStream;
        int m_field04;
        int m_field08;
        unsigned int m_field0c;
    };

    PacketStream();
    Result Initialize(unsigned int packetNum, unsigned int field28);
    void Finalize();
    Result Startup();
    void Cleanup();
    void Trace(unsigned long long value) const;

    unsigned int m_packetNum;
    common::Packet* m_packets;
    Writer m_writer;
    Reader m_reader;
    unsigned int m_field28;
};

}}}

#endif
