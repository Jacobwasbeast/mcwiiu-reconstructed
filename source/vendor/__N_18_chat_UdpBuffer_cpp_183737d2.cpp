#include "vendor/__N_18_chat_UdpBuffer_cpp_183737d2.hpp"

#include <cstring>

struct _udp_packet
{
    _udp_packet *nextPacket;
    unsigned char contents[3532];
};

namespace
{
    _udp_packet *packetBuffer = 0;
    _udp_packet *freePacketBuffer = 0;
    unsigned int packetCount = 0;
    unsigned int packetBufferSize = 0;
}

void __N_18_chat_UdpBuffer_cpp_183737d2::initImpl()
{
    std::memset(packetBuffer, 0, packetBufferSize);
    if (packetCount != 1)
    {
        unsigned int index = 0;
        do
        {
            packetBuffer[index].nextPacket = &packetBuffer[index + 1];
            ++index;
        }
        while (index < packetCount - 1);
    }
    packetBuffer[packetCount - 1].nextPacket = 0;
    freePacketBuffer = packetBuffer;
}

void __N_18_chat_UdpBuffer_cpp_183737d2::trace_udp_packet(unsigned long long timestamp, const _udp_packet& packet)
{
    (void)timestamp;
    (void)packet;
}
