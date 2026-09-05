#ifndef SOURCE_PLATFORM_WIIU_FFLIRESOURCEPARTSINFO_HPP
#define SOURCE_PLATFORM_WIIU_FFLIRESOURCEPARTSINFO_HPP

#include <stdint.h>

enum FFLiResourceWindowBits
{
    FFLiResourceWindowBits_0 = 0
};

struct FFLiResourcePartsInfo
{
    uint32_t offset;
    uint32_t uncompressedSize;
    uint32_t compressedSize;
    uint8_t partType;
    uint8_t windowBits;
    uint8_t flags;
    uint8_t compressionType;

    FFLiResourceWindowBits ZlibWindowBits() const;
};

#endif
