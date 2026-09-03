#include "platform/wiiu/FFLiResourceHeader.hpp"

#include <string.h>

uint32_t FFLiCheckAlignPtr(const void* pointer, unsigned int alignment);

static uint32_t FFLiSwapWord(uint32_t value)
{
    return ((value & 0x000000ffU) << 24) |
           ((value & 0x0000ff00U) << 8) |
           ((value & 0x00ff0000U) >> 8) |
           ((value & 0xff000000U) >> 24);
}

uint32_t FFLiResourceHeader::GetResult() const
{
    if (FFLiCheckAlignPtr(this, 4) == 0)
    {
        return 1;
    }
    if ((magic == 0x46465241U) && (version == 0x00070000U))
    {
        return 0;
    }
    return 3;
}

void FFLiResourceHeader::SwapEndian()
{
    uint32_t* words = &magic;
    unsigned int index = 0;
    while (index < 4736U)
    {
        words[index] = FFLiSwapWord(words[index]);
        ++index;
    }
}

uint32_t FFLiResourceHeader::GetExpandedBufferSize(unsigned int resourceType) const
{
    if (resourceType == 0xffffffffU)
    {
        return uncompressBufferSize;
    }
    if (resourceType < 2U)
    {
        const uint32_t size = resourceWords[resourceType];
        if (size != 0U)
        {
            return size;
        }
    }
    return uncompressBufferSize;
}

uint32_t FFLiResourceHeader::Expand(void* destination, unsigned int destinationSize, unsigned int resourceType, FFLExpandBuffer* expandBuffer) const
{
    const uint32_t requiredSize = GetExpandedBufferSize(resourceType);
    if (destination == 0 || destinationSize < requiredSize)
    {
        return 1;
    }
    if (expandBuffer == 0)
    {
        memcpy(destination, this, requiredSize);
        return 0;
    }
    memcpy(destination, this, requiredSize);
    return 0;
}
