#ifndef PLATFORM_WIIU_FFLIZLIBALLOCATOR_HPP
#define PLATFORM_WIIU_FFLIZLIBALLOCATOR_HPP

#include "platform/wiiu/FFLiAllocatorInterface.hpp"

class FFLiZlibAllocator
{
public:
    FFLiZlibAllocator(FFLiAllocatorInterface& allocator);

    static void Allocate(void* opaque, unsigned int items, unsigned int size);
    static void Free(void* opaque, void* buffer);

private:
    FFLiAllocatorInterface* mAllocator;
};

#endif
