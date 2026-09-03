#include "vendor/_MapDataMappings_old.hpp"

#include <cstring>

_MapDataMappings_old::_MapDataMappings_old()
{
    std::memset(mMappingWords, 0, 0x500);
    std::memset(mDimensionMappings, 0, 8);
}

unsigned int _MapDataMappings_old::getDimension(int mapping)
{
    const unsigned int byteIndex = static_cast<unsigned int>(mapping >> 3);
    const unsigned int bitIndex = static_cast<unsigned int>(mapping & 7);
    if ((mDimensionMappings[byteIndex] & (static_cast<unsigned char>(0x80) >> bitIndex)) == 0)
    {
        return 0;
    }
    return 0xffffffffU;
}

void _MapDataMappings_old::setMapping(int mapping, PlayerUID player, int dimension)
{
    std::memcpy(mMappingWords + mapping * 5, &player, 20);
    const unsigned int byteIndex = static_cast<unsigned int>(mapping >> 3);
    const unsigned char bit = static_cast<unsigned char>(0x80) >> static_cast<unsigned int>(mapping & 7);
    if (dimension == 0)
    {
        mDimensionMappings[byteIndex] = static_cast<unsigned char>(mDimensionMappings[byteIndex] & static_cast<unsigned char>(~bit));
    }
    else
    {
        mDimensionMappings[byteIndex] = static_cast<unsigned char>(mDimensionMappings[byteIndex] | bit);
    }
}
