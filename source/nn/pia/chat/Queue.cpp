#include "vendor/VCTQueue.hpp"

#include <coreinit/fastmutex.h>

namespace
{
    const unsigned int AUDIO_QUEUE_MAX = 15;

    unsigned int audioQueueCounts[AUDIO_QUEUE_MAX] = { 0 };
    _udp_packet* audioQueueHeads[AUDIO_QUEUE_MAX] = { 0 };
    _udp_packet* audioQueueTails[AUDIO_QUEUE_MAX] = { 0 };

    OSFastMutex audioQueueMutex;

    void flushQueue(unsigned int streamIndex)
    {
        _udp_packet* packet = audioQueueHeads[streamIndex];
        while (packet != 0)
        {
            _udp_packet* nextPacket = packet->m_pNext;
            audioQueueHeads[streamIndex] = nextPacket;
            if (nextPacket != 0)
            {
                nextPacket->m_pPrevious = 0;
            }
            vct_free_packet_buffer(packet);
            packet = audioQueueHeads[streamIndex];
        }
        audioQueueTails[streamIndex] = 0;
        audioQueueCounts[streamIndex] = 0;
    }
}

void vct_init_audio_queue(unsigned int uiStreamIndex)
{
    OSFastMutex_Lock(&audioQueueMutex);
    audioQueueHeads[uiStreamIndex] = 0;
    audioQueueTails[uiStreamIndex] = 0;
    audioQueueCounts[uiStreamIndex] = 0;
    OSFastMutex_Unlock(&audioQueueMutex);
}

int vct_insert_audio_queue(_udp_packet* pPacket, unsigned int uiStreamIndex)
{
    if (pPacket == 0 || uiStreamIndex >= AUDIO_QUEUE_MAX)
    {
        return -1;
    }

    OSFastMutex_Lock(&audioQueueMutex);

    if (audioQueueHeads[uiStreamIndex] == 0)
    {
        audioQueueHeads[uiStreamIndex] = pPacket;
        pPacket->m_pPrevious = 0;
        audioQueueHeads[uiStreamIndex]->m_pNext = 0;
        const unsigned int uiCount = audioQueueCounts[uiStreamIndex] + 1;
        audioQueueTails[uiStreamIndex] = audioQueueHeads[uiStreamIndex];
        audioQueueCounts[uiStreamIndex] = uiCount;
        OSFastMutex_Unlock(&audioQueueMutex);
        return static_cast<int>(uiCount);
    }

    _udp_packet* pCursor = audioQueueTails[uiStreamIndex];
    if (audioQueueHeads[uiStreamIndex]->m_pPrevious != 0 || pCursor->m_pNext != 0)
    {
        vct_trace_packet_buffer(static_cast<unsigned long long>(0x40) << 32);
        pCursor = audioQueueTails[uiStreamIndex];
    }

    while (pCursor != 0)
    {
        if (pCursor->m_Sequence < pPacket->m_Sequence)
        {
            pPacket->m_pPrevious = pCursor;
            pPacket->m_pNext = pCursor->m_pNext;
            pCursor->m_pNext = pPacket;
            if (pPacket->m_pNext != 0)
            {
                pPacket->m_pNext->m_pPrevious = pPacket;
            }
            if (pCursor == audioQueueTails[uiStreamIndex])
            {
                audioQueueTails[uiStreamIndex] = pPacket;
            }
            const unsigned int uiCount = audioQueueCounts[uiStreamIndex] + 1;
            audioQueueCounts[uiStreamIndex] = uiCount;
            OSFastMutex_Unlock(&audioQueueMutex);
            return static_cast<int>(uiCount);
        }
        pCursor = pCursor->m_pPrevious;
    }

    pPacket->m_pPrevious = 0;
    pPacket->m_pNext = audioQueueHeads[uiStreamIndex];
    audioQueueHeads[uiStreamIndex]->m_pPrevious = pPacket;
    audioQueueHeads[uiStreamIndex] = pPacket;
    const unsigned int uiCount = audioQueueCounts[uiStreamIndex] + 1;
    audioQueueCounts[uiStreamIndex] = uiCount;
    OSFastMutex_Unlock(&audioQueueMutex);
    return static_cast<int>(uiCount);
}

_udp_packet* vct_top_audio_queue(unsigned int uiStreamIndex)
{
    OSFastMutex_Lock(&audioQueueMutex);
    _udp_packet* pPacket = audioQueueHeads[uiStreamIndex];
    OSFastMutex_Unlock(&audioQueueMutex);
    return pPacket;
}

int vct_pop_audio_queue(unsigned int uiStreamIndex)
{
    OSFastMutex_Lock(&audioQueueMutex);

    unsigned int uiCount = audioQueueCounts[uiStreamIndex];
    _udp_packet* pHead = audioQueueHeads[uiStreamIndex];
    if (pHead != 0)
    {
        uiCount = uiCount - 1;
        _udp_packet* pNext = pHead->m_pNext;
        audioQueueCounts[uiStreamIndex] = uiCount;
        audioQueueHeads[uiStreamIndex] = pNext;
        if (pNext != 0)
        {
            pNext->m_pPrevious = 0;
            uiCount = audioQueueCounts[uiStreamIndex];
            OSFastMutex_Unlock(&audioQueueMutex);
        }
        else
        {
            audioQueueTails[uiStreamIndex] = 0;
            OSFastMutex_Unlock(&audioQueueMutex);
        }
    }
    else
    {
        OSFastMutex_Unlock(&audioQueueMutex);
    }

    return static_cast<int>(uiCount);
}

int vct_count_audio_queue(unsigned int uiStreamIndex)
{
    OSFastMutex_Lock(&audioQueueMutex);
    const unsigned int uiCount = audioQueueCounts[uiStreamIndex];
    OSFastMutex_Unlock(&audioQueueMutex);
    return static_cast<int>(uiCount);
}

void vct_flush_audio_queue(unsigned int uiStreamIndex)
{
    OSFastMutex_Lock(&audioQueueMutex);
    flushQueue(uiStreamIndex);
    OSFastMutex_Unlock(&audioQueueMutex);
}

void vct_flush_all_audio_queue()
{
    OSFastMutex_Lock(&audioQueueMutex);
    for (unsigned int uiStream = 0; uiStream < AUDIO_QUEUE_MAX; ++uiStream)
    {
        flushQueue(uiStream);
    }
    OSFastMutex_Unlock(&audioQueueMutex);
}
