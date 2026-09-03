#include "vendor/_MapDataMappings.hpp"

#include <cstring>

_MapDataMappings::_MapDataMappings()
{
    std::memset(m_playerUidData, 0, sizeof(m_playerUidData));
    std::memset(m_dimensions, 0, sizeof(m_dimensions));
}

unsigned int _MapDataMappings::getDimension(int mapId)
{
    const unsigned int shift = static_cast<unsigned int>((mapId & 3) * 2);
    const unsigned int value = (m_dimensions[mapId >> 2] >> shift) & 3;
    if (value == 1)
    {
        return static_cast<unsigned int>(-1);
    }
    if (value == 2)
    {
        return 1;
    }
    return 0;
}

void _MapDataMappings::setMapping(int mapId, PlayerUID uid, int dimension)
{
    std::memcpy(m_playerUidData[mapId], &uid, sizeof(uid));
    const unsigned int shift = static_cast<unsigned int>((mapId & 3) * 2);
    unsigned int value = 0;
    if (dimension == -1)
    {
        value = 1;
    }
    else if (dimension == 1)
    {
        value = 2;
    }
    m_dimensions[mapId >> 2] = static_cast<unsigned char>((m_dimensions[mapId >> 2] & ~(3u << shift)) | (value << shift));
}
