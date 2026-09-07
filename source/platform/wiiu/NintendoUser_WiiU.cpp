#include "platform/wiiu/NintendoUser_WiiU.hpp"

extern "C" void* MEMAllocFromDefaultHeapEx(unsigned int size, int alignment);
extern "C" void MEMFreeToDefaultHeap(void* block);

int CoreUser_DynLoad_DefaultAlloc__FiT1PPv(int32_t size, int32_t alignment, void** result)
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

void CoreUser_DynLoad_DefaultFree__FPv(void* block)
{
    MEMFreeToDefaultHeap(block);
}
