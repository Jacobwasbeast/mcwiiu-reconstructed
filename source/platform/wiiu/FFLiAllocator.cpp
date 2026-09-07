#include "platform/wiiu/FFLiAllocator.hpp"

extern "C"
{
void* MEMCreateExpHeapEx(void* address, unsigned int size, unsigned short flags);
void MEMDestroyExpHeap(void* heap);
void* MEMAllocFromExpHeapEx(void* heap, unsigned int size, int alignment);
void MEMFreeToExpHeap(void* heap, void* block);
}

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
