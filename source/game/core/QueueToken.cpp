#include "game/core/QueueToken.hpp"
#include "game/client/audio/FalloffGraph.hpp"

extern "C" unsigned int rrAtomicLoadAcquire32(const volatile unsigned int* address);
extern "C" unsigned int rrAtomicCmpXchg32(volatile unsigned int* address,
                                          unsigned int value,
                                          unsigned int comparand);
extern "C" void rrThreadSleep(unsigned int milliseconds);

void ClearQueueToken(int token)
{
    const unsigned int mask = 1u << (token & 0x3f);
    unsigned int current = rrAtomicLoadAcquire32(&g_eventQueue.tokens);
    unsigned int previous = rrAtomicCmpXchg32(&g_eventQueue.tokens, current & ~mask, current);

    if (previous != current)
    {
        do
        {
            rrThreadSleep(0);
            current = rrAtomicLoadAcquire32(&g_eventQueue.tokens);
            previous = rrAtomicCmpXchg32(&g_eventQueue.tokens, current & ~mask, current);
        }
        while (previous != current);
    }
}
