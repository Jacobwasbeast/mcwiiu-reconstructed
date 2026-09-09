#include "vendor/__N_18_chat_UdpBuffer_cpp_183737d2.hpp"

#include "vendor/VCTQueue.hpp"

#include <coreinit/fastmutex.h>

#include <cstring>

namespace
{
    const unsigned int PACKET_BUFFER_COUNT_MIN = 4;
    const unsigned int PACKET_BUFFER_COUNT_MAX = 0x168;
    const unsigned long PACKET_BUFFER_ALIGNMENT_MASK = 0x1f;

    _udp_packet* packetBuffer = 0;
    _udp_packet* freePacketBuffer = 0;
    unsigned int packetCount = 0;
    unsigned int packetBufferSize = 0;

    OSFastMutex packetBufferMutex;
}

void __N_18_chat_UdpBuffer_cpp_183737d2::initImpl()
{
    std::memset(packetBuffer, 0, packetBufferSize);
    for (unsigned int index = 0; index < packetCount - 1; ++index)
    {
        packetBuffer[index].m_pNextFree = &packetBuffer[index + 1];
    }
    packetBuffer[packetCount - 1].m_pNextFree = 0;
    freePacketBuffer = packetBuffer;
}

int vct_init_packet_buffer(void* pBuffer, unsigned int uiSize)
{
    OSFastMutex_Lock(&packetBufferMutex);

    const unsigned int uiCount = uiSize / sizeof(_udp_packet);
    if (pBuffer != 0 &&
        (reinterpret_cast<unsigned long>(pBuffer) & PACKET_BUFFER_ALIGNMENT_MASK) == 0 &&
        uiCount >= PACKET_BUFFER_COUNT_MIN && uiCount <= PACKET_BUFFER_COUNT_MAX)
    {
        packetBufferSize = uiSize;
        packetBuffer = static_cast<_udp_packet*>(pBuffer);
        packetCount = uiCount;
        __N_18_chat_UdpBuffer_cpp_183737d2::initImpl();
        OSFastMutex_Unlock(&packetBufferMutex);
        return 1;
    }

    OSFastMutex_Unlock(&packetBufferMutex);
    return 0;
}

void vct_init_packet_buffer_ex()
{
    OSFastMutex_Lock(&packetBufferMutex);
    __N_18_chat_UdpBuffer_cpp_183737d2::initImpl();
    OSFastMutex_Unlock(&packetBufferMutex);
}

void vct_cleanup_packet_buffer()
{
    OSFastMutex_Lock(&packetBufferMutex);
    packetBuffer = 0;
    freePacketBuffer = 0;
    OSFastMutex_Unlock(&packetBufferMutex);
}

_udp_packet* vct_alloc_packet_buffer()
{
    OSFastMutex_Lock(&packetBufferMutex);

    _udp_packet* pPacket = 0;
    if (freePacketBuffer != 0)
    {
        pPacket = freePacketBuffer;
        freePacketBuffer = freePacketBuffer->m_pNextFree;
    }

    OSFastMutex_Unlock(&packetBufferMutex);
    return pPacket;
}

void vct_free_packet_buffer(_udp_packet* pPacket)
{
    if (pPacket == 0)
    {
        return;
    }

    OSFastMutex_Lock(&packetBufferMutex);
    pPacket->m_pNextFree = freePacketBuffer;
    freePacketBuffer = pPacket;
    pPacket->m_pNext = 0;
    pPacket->m_pPrevious = 0;
    OSFastMutex_Unlock(&packetBufferMutex);
}

void vct_trace_packet_buffer(unsigned long long)
{
    OSFastMutex_Lock(&packetBufferMutex);
    OSFastMutex_Unlock(&packetBufferMutex);
}
