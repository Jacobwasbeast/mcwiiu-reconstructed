#include "nn/pia/transport/PacketStream.hpp"

namespace nn { namespace pia { namespace transport {

PacketStream::PacketStream()
    : m_packetNum(0), m_packets(0), m_writer(this), m_reader(this)
{
}

Result PacketStream::Startup()
{
    if (m_packets == 0)
        return static_cast<Result>(0x10c0c);

    m_writer.m_field04 = 0;
    m_writer.m_field08 = 0;
    m_writer.m_field0c = 0;
    m_reader.m_field04 = 0;
    m_reader.m_field08 = 0;
    m_reader.m_field0c = 0;
    return 0;
}

void PacketStream::Cleanup()
{
    m_writer.m_field04 = -1;
    m_writer.m_field08 = -1;
    m_writer.m_field0c = 0;
    m_reader.m_field04 = -1;
    m_reader.m_field08 = -1;
    m_reader.m_field0c = 0;
}

void PacketStream::Trace(unsigned long long value) const
{
}

}}}
