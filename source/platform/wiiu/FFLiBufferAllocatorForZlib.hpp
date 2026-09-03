#ifndef PLATFORM_WIIU_FFLIBUFFERALLOCATORFORZLIB_HPP
#define PLATFORM_WIIU_FFLIBUFFERALLOCATORFORZLIB_HPP

#include "platform/wiiu/FFLiAllocatorInterface.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"

class FFLiBufferAllocatorForZlib : public FFLiAllocatorInterface
{
public:
    FFLiBufferAllocatorForZlib(void* buffer, unsigned int size);
    virtual ~FFLiBufferAllocatorForZlib();
    virtual void Allocate(unsigned int size);
    virtual void* Allocate(unsigned int size, unsigned int alignment);
    virtual void Free(void* buffer);

private:
    FFLiBufferAllocator mAllocator;
};

#endif
