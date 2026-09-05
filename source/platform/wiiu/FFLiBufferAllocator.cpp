#include "platform/wiiu/FFLiBufferAllocator.hpp"

unsigned int FFLiRoundUp(unsigned int value, unsigned int alignment);

void FFLiBufferAllocator::Init(void* buffer, unsigned int size)
{
    mBuffer = (unsigned int)buffer;
    mRestSize = size;
}

unsigned int FFLiBufferAllocator::Allocate(unsigned int size, unsigned int alignment)
{
    unsigned int current = mBuffer;
    unsigned int aligned = (current + alignment - 1U) & ~(alignment - 1U);
    mBuffer = aligned + size;
    mRestSize -= (aligned - current) + size;
    return aligned;
}

unsigned int FFLiBufferAllocator::Allocate(unsigned int size)
{
    return Allocate(size, 1U);
}

unsigned int FFLiBufferAllocator::GetRestSize() const
{
    return mRestSize;
}

bool FFLiBufferAllocator::CanAllocate(unsigned int size, unsigned int alignment) const
{
    unsigned int current = mBuffer;
    unsigned int aligned = FFLiRoundUp(current, alignment);
    return aligned + size - current <= GetRestSize();
}
