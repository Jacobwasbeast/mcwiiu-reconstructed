#include "vendor/_MapDataMappings.hpp"

#include <cstring>

_MapDataMappings::_MapDataMappings()
{
    std::memset(xuids, 0, sizeof(xuids));
    std::memset(dimensions, 0, sizeof(dimensions));
}

int _MapDataMappings::getDimension(int id)
{
    int shift = (id % 4) * 2;
    int value = (dimensions[id >> 2] & (3 << shift)) >> shift;
    switch (value)
    {
    case 0:
        return 0;
    case 1:
        return -1;
    case 2:
        return 1;
    default:
        return 0;
    }
}

void _MapDataMappings::setMapping(int id, PlayerUID xuid, int dimension)
{
    xuids[id] = xuid;

    int shift = (id % 4) * 2;
    dimensions[id >> 2] &= ~(3 << shift);
    switch (dimension)
    {
    case -1:
        dimensions[id >> 2] |= 1 << shift;
        break;
    case 1:
        dimensions[id >> 2] |= 2 << shift;
        break;
    default:
        break;
    }
}
