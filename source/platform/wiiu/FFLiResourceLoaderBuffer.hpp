#ifndef SOURCE_PLATFORM_WIIU_FFLIRESOURCELOADERBUFFER_HPP
#define SOURCE_PLATFORM_WIIU_FFLIRESOURCELOADERBUFFER_HPP

#include "platform/wiiu/FFLiResourceUncompressBuffer.hpp"

class FFLiResourceLoaderBuffer : public FFLiResourceUncompressBuffer
{
    friend class FFLiResourceLoader;
private:
    unsigned int mResourceBuffer;

public:
    FFLiResourceLoaderBuffer(const FFLiResourceManager* resourceManager,
                             FFLiBufferAllocator* allocator,
                             FFLResourceType resourceType);
    ~FFLiResourceLoaderBuffer();
    static int GetBufferSize(const FFLiResourceManager* resourceManager,
                             FFLResourceType resourceType);
};

#endif
