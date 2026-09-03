#include "platform/wiiu/FFLiZlibAllocator.hpp"

FFLiZlibAllocator::FFLiZlibAllocator(FFLiAllocatorInterface& allocator)
    : mAllocator(&allocator)
{
}

void FFLiZlibAllocator::Allocate(void* opaque, unsigned int items, unsigned int size)
{
    FFLiZlibAllocator* allocator = static_cast<FFLiZlibAllocator*>(opaque);
    allocator->mAllocator->Allocate(items * size);
}

void FFLiZlibAllocator::Free(void* opaque, void* buffer)
{
    FFLiZlibAllocator* allocator = static_cast<FFLiZlibAllocator*>(opaque);
    allocator->mAllocator->Free(buffer);
}
