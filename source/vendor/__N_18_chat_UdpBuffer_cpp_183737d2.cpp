#include "vendor/__N_18_chat_UdpBuffer_cpp_183737d2.hpp"

#include "vendor/VCTQueue.hpp"

#include <coreinit/fastmutex.h>

#include <cstring>

namespace
{
    const unsigned int PACKET_BUFFER_COUNT_MIN = 4;
    const unsigned int PACKET_BUFFER_COUNT_RANGE = 0x165;
    const unsigned long PACKET_BUFFER_ALIGNMENT_MASK = 0x1f;

    _udp_packet* packetBuffer = 0;
    _udp_packet* freePacketBuffer = 0;
    unsigned int packetCount = 0;
    unsigned int packetBufferSize = 0;

    OSFastMutex packetBufferMutex;
}

namespace __N_18_chat_UdpBuffer_cpp_183737d2
{
    void initImpl()
    {
        std::memset(packetBuffer, 0, packetBufferSize);
        for (unsigned int index = 0; index < packetCount - 1; ++index)
        {
            packetBuffer[index].m_pNextFree = &packetBuffer[index + 1];
        }
        packetBuffer[packetCount - 1].m_pNextFree = 0;
        freePacketBuffer = packetBuffer;
    }
}

namespace __N_18_chat_UdpBuffer_cpp_183737d2
{
    void trace_udp_packet(unsigned long long, const _udp_packet&)
    {
    }
}

int vct_init_packet_buffer(void* pBuffer, unsigned int uiSize)
{
    unsigned int uiCount;
    OSFastMutex_Lock(&packetBufferMutex);

    if (pBuffer == 0 ||
        (reinterpret_cast<unsigned long>(pBuffer) & PACKET_BUFFER_ALIGNMENT_MASK) != 0 ||
        (uiCount = uiSize / sizeof(_udp_packet)) - PACKET_BUFFER_COUNT_MIN >= PACKET_BUFFER_COUNT_RANGE)
    {
        OSFastMutex_Unlock(&packetBufferMutex);
        return 0;
    }

    packetBuffer = static_cast<_udp_packet*>(pBuffer);
    packetCount = uiCount;
    packetBufferSize = uiSize;
    __N_18_chat_UdpBuffer_cpp_183737d2::initImpl();
    OSFastMutex_Unlock(&packetBufferMutex);
    return 1;
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
    packetBuffer = freePacketBuffer = 0;
    OSFastMutex_Unlock(&packetBufferMutex);
}

_udp_packet* vct_alloc_packet_buffer()
{
    _udp_packet* pPacket = 0;
    OSFastMutex_Lock(&packetBufferMutex);

    _udp_packet* pFree = freePacketBuffer;
    if (pFree != 0)
    {
        pPacket = pFree;
        freePacketBuffer = pFree->m_pNextFree;
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
    pPacket->m_pPrevious = 0;
    pPacket->m_pNext = 0;
    OSFastMutex_Unlock(&packetBufferMutex);
}

void vct_trace_packet_buffer(unsigned long long ullTag)
{
    OSFastMutex_Lock(&packetBufferMutex);
    for (unsigned int index = 0; index < packetCount; ++index)
    {
        __N_18_chat_UdpBuffer_cpp_183737d2::trace_udp_packet(ullTag, packetBuffer[index]);
    }
    OSFastMutex_Unlock(&packetBufferMutex);
}
