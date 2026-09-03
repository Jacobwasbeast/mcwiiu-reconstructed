#include "vendor/__N_14_chat_Queue_cpp_8071bfa5.hpp"

struct _udp_packet
{
    unsigned int packetHeader;
    _udp_packet *previousPacket;
    _udp_packet *nextPacket;
};

void vct_free_packet_buffer__FP11_udp_packet(_udp_packet *packet);

namespace
{
    _udp_packet *audioQueueHeads[15] = { 0 };
    _udp_packet *audioQueueTails[15] = { 0 };
    unsigned int audioQueueCounts[15] = { 0 };
}

void __N_14_chat_Queue_cpp_8071bfa5::flushQueue(unsigned int streamIndex)
{
    _udp_packet *packet = audioQueueHeads[streamIndex];
    while (packet != 0)
    {
        _udp_packet *nextPacket = packet->nextPacket;
        audioQueueHeads[streamIndex] = nextPacket;
        if (nextPacket != 0)
        {
            nextPacket->previousPacket = 0;
        }
        vct_free_packet_buffer__FP11_udp_packet(packet);
        packet = audioQueueHeads[streamIndex];
    }
    audioQueueTails[streamIndex] = 0;
    audioQueueCounts[streamIndex] = 0;
}
