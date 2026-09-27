#include "platform/wiiu/NintendoUser_WiiU.hpp"

#include <coreinit/memdefaultheap.h>

int CoreUser_DynLoad_DefaultAlloc(int32_t size, int32_t alignment, void** result)
{
    void* block;

    if (result == 0)
    {
        return 0xbad10017;
    }

    if (alignment < 0)
    {
        if (alignment > -4)
        {
            block = MEMAllocFromDefaultHeapEx((unsigned int)size, -4);
            *result = block;
        }
        else
        {
            block = MEMAllocFromDefaultHeapEx((unsigned int)size, alignment);
            *result = block;
        }
    }
    else
    {
        if (alignment < 4)
        {
            alignment = 4;
        }

        block = MEMAllocFromDefaultHeapEx((unsigned int)size, alignment);
        *result = block;
    }

    return block != 0 ? 0 : 0xbad10002;
}

void CoreUser_DynLoad_DefaultFree(void* block)
{
    MEMFreeToDefaultHeap(block);
}
