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
    class Accessor
    {
    public:
        Accessor(PacketStream* packetStream)
            : m_packetStream(packetStream), m_field04(-1), m_field08(-1), m_field0c(0)
        {
        }
        common::Packet* Get(int index);

        PacketStream* m_packetStream;
        int m_field04;
        int m_field08;
        unsigned int m_field0c;
    };

    class Writer : public Accessor
    {
    public:
        Writer(PacketStream* packetStream) : Accessor(packetStream)
        {
        }
        int AssignableNum() const;
        common::Packet* Assign();
        void Push();
    };

    class Reader : public Accessor
    {
    public:
        Reader(PacketStream* packetStream) : Accessor(packetStream)
        {
        }
        void PullAll();
        void Release();
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
