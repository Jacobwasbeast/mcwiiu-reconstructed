#include "game/core/QueueToken.hpp"

extern unsigned int rrAtomicLoadAcquire32(const volatile unsigned int* address);
extern unsigned int rrAtomicCmpXchg32(volatile unsigned int* address,
                                      unsigned int value,
                                      unsigned int comparand);
extern void rrThreadSleep(unsigned int milliseconds);

static volatile unsigned int s_queueTokens = 0;

void ClearQueueToken__Fi(int token)
{
    const unsigned int mask = 1u << (token & 0x3f);
    unsigned int current = rrAtomicLoadAcquire32(&s_queueTokens);
    unsigned int previous = rrAtomicCmpXchg32(&s_queueTokens, current & ~mask, current);

    if (previous != current)
    {
        do
        {
            rrThreadSleep(0);
            current = rrAtomicLoadAcquire32(&s_queueTokens);
            previous = rrAtomicCmpXchg32(&s_queueTokens, current & ~mask, current);
        }
        while (previous != current);
    }
}
