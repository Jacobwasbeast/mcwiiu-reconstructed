#include "platform/wiiu/FFLiBufferAllocatorForZlib.hpp"

FFLiBufferAllocatorForZlib::FFLiBufferAllocatorForZlib(void* buffer, unsigned int size)
{
    mAllocator.Init(buffer, size);
}

FFLiBufferAllocatorForZlib::~FFLiBufferAllocatorForZlib()
{
}

void FFLiBufferAllocatorForZlib::Allocate(unsigned int size)
{
    mAllocator.Allocate(size);
}

void* FFLiBufferAllocatorForZlib::Allocate(unsigned int size, unsigned int alignment)
{
    return (void*)mAllocator.Allocate(size, alignment);
}

void FFLiBufferAllocatorForZlib::Free(void* buffer)
{
}
