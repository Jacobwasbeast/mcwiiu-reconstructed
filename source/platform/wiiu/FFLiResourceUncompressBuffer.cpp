#include "platform/wiiu/FFLiResourceUncompressBuffer.hpp"

#include "platform/wiiu/FFLiBufferAllocator.hpp"

FFLiResourceUncompressBuffer::FFLiResourceUncompressBuffer(
    const FFLiResourceManager* resourceManager,
    FFLiBufferAllocator* allocator,
    FFLResourceType resourceType)
{
    mBuffer = allocator->Allocate(resourceManager->GetUncompressBufferSize(resourceType), 64);
    mTempBuffer = allocator->Allocate(GetTempBufferSize(), 1);
}

FFLiResourceUncompressBuffer::~FFLiResourceUncompressBuffer()
{
}

unsigned int FFLiResourceUncompressBuffer::Buffer()
{
    return mBuffer;
}

unsigned int FFLiResourceUncompressBuffer::Buffer() const
{
    return mBuffer;
}

unsigned int FFLiResourceUncompressBuffer::TempBuffer()
{
    return mTempBuffer;
}

unsigned int FFLiResourceUncompressBuffer::TempBufferSize() const
{
    return GetTempBufferSize();
}

int FFLiResourceUncompressBuffer::GetBufferSize(
    const FFLiResourceManager* resourceManager,
    FFLResourceType resourceType)
{
    return static_cast<int>(resourceManager->GetUncompressBufferSize(resourceType) + GetTempBufferSize() + 64);
}

unsigned int FFLiResourceUncompressBuffer::GetTempBufferSize()
{
    return 0x19000;
}
