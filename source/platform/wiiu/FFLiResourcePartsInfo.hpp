#ifndef SOURCE_PLATFORM_WIIU_FFLIRESOURCEPARTSINFO_HPP
#define SOURCE_PLATFORM_WIIU_FFLIRESOURCEPARTSINFO_HPP

#include <stdint.h>

enum FFLiResourceWindowBits
{
    FFLiResourceWindowBits_0 = 0,
    FFLiResourceWindowBits_1 = 1,
    FFLiResourceWindowBits_2 = 2,
    FFLiResourceWindowBits_3 = 3,
    FFLiResourceWindowBits_4 = 4,
    FFLiResourceWindowBits_5 = 5,
    FFLiResourceWindowBits_6 = 6,
    FFLiResourceWindowBits_7 = 7,
    FFLiResourceWindowBits_8 = 8,
    FFLiResourceWindowBits_9 = 9,
    FFLiResourceWindowBits_10 = 10,
    FFLiResourceWindowBits_11 = 11,
    FFLiResourceWindowBits_12 = 12,
    FFLiResourceWindowBits_13 = 13,
    FFLiResourceWindowBits_14 = 14,
    FFLiResourceWindowBits_15 = 15,
    FFLiResourceWindowBits_16 = 16
};

struct FFLiResourcePartsInfo
{
    uint32_t offset;
    uint32_t uncompressedSize;
    uint32_t compressedSize;
    uint8_t compressLevel;
    uint8_t windowBits;
    uint8_t memoryLevel;
    uint8_t strategy;

    int ZlibWindowBits() const;
};

#endif
