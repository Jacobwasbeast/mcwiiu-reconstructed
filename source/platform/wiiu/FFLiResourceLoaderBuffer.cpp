#include "platform/wiiu/FFLiResourceLoaderBuffer.hpp"

#include "platform/wiiu/FFLiBufferAllocator.hpp"

FFLiResourceLoaderBuffer::FFLiResourceLoaderBuffer(
    const FFLiResourceManager* resourceManager,
    FFLiBufferAllocator* allocator,
    FFLResourceType resourceType)
    : FFLiResourceUncompressBuffer(resourceManager, allocator, resourceType)
{
    mResourceBuffer = allocator->Allocate(0xA84, 1U);
}

FFLiResourceLoaderBuffer::~FFLiResourceLoaderBuffer()
{
}

int FFLiResourceLoaderBuffer::GetBufferSize(
    const FFLiResourceManager* resourceManager,
    FFLResourceType resourceType)
{
    return FFLiResourceUncompressBuffer::GetBufferSize(resourceManager, resourceType) + 0xA84;
}
