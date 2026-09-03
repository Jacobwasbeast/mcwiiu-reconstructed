#ifndef PLATFORM_WIIU_FFLIRESOURCEUNCOMPRESSBUFFER_HPP
#define PLATFORM_WIIU_FFLIRESOURCEUNCOMPRESSBUFFER_HPP

#include "platform/wiiu/FFLiResourceManager.hpp"

class FFLiBufferAllocator;

class FFLiResourceUncompressBuffer
{
public:
    FFLiResourceUncompressBuffer(const FFLiResourceManager* resourceManager,
                                 FFLiBufferAllocator* allocator,
                                 FFLResourceType resourceType);
    ~FFLiResourceUncompressBuffer();

    unsigned int Buffer();
    unsigned int Buffer() const;
    unsigned int TempBuffer();
    unsigned int TempBufferSize() const;
    static int GetBufferSize(const FFLiResourceManager* resourceManager,
                             FFLResourceType resourceType);
    static unsigned int GetTempBufferSize();

private:
    unsigned int mBuffer;
    unsigned int mTempBuffer;
};

#endif
