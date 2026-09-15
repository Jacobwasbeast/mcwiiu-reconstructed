#include "platform/wiiu/FFLiResourceHeader.hpp"

#include <string.h>

bool FFLiCheckAlignPtr(const void* pointer, unsigned int alignment);

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
