#include "platform/wiiu/FFLiAllocator.hpp"
#include <coreinit/memexpheap.h>

FFLiAllocator::FFLiAllocator()
{
    mHeap = 0;
}

FFLiAllocator::~FFLiAllocator()
{
    if (IsValid())
    {
        MEMDestroyExpHeap(mHeap);
    }
}

bool FFLiAllocator::IsValid() const
{
    return mHeap != 0;
}

void FFLiAllocator::Init(void* buffer, unsigned int size)
{
    mHeap = MEMCreateExpHeapEx(buffer, size, 0);
}

void* FFLiAllocator::Allocate(unsigned int size, unsigned int alignment)
{
    return MEMAllocFromExpHeapEx(mHeap, size, (int)alignment);
}

void FFLiAllocator::Free(void* block)
{
    MEMFreeToExpHeap(mHeap, block);
}
