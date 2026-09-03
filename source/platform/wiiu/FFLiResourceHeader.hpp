#ifndef PLATFORM_WIIU_FFLIRESOURCEHEADER_HPP
#define PLATFORM_WIIU_FFLIRESOURCEHEADER_HPP

#include <stdint.h>

struct FFLExpandBuffer;

struct FFLiResourceHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t uncompressBufferSize;
    uint32_t resourceWords[4733];

    uint32_t GetResult() const;
    void SwapEndian();
    uint32_t GetExpandedBufferSize(unsigned int resourceType) const;
    uint32_t Expand(void* destination, unsigned int destinationSize, unsigned int resourceType, FFLExpandBuffer* expandBuffer) const;
};

#endif
