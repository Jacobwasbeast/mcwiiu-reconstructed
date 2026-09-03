#ifndef VENDOR__MAPDATAMAPPINGS_HPP
#define VENDOR__MAPDATAMAPPINGS_HPP

#include "game/world/generation/PlayerUID.hpp"

class _MapDataMappings
{
public:
    _MapDataMappings();

    unsigned int getDimension(int mapId);
    void setMapping(int mapId, PlayerUID uid, int dimension);

private:
    unsigned char m_playerUidData[256][20];
    unsigned char m_dimensions[16];
};

#endif
