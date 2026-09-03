#ifndef SOURCE_VENDOR_MAPDATAMAPPINGS_OLD_HPP
#define SOURCE_VENDOR_MAPDATAMAPPINGS_OLD_HPP

#include "game/world/generation/PlayerUID.hpp"

class _MapDataMappings_old
{
public:
    _MapDataMappings_old();
    unsigned int getDimension(int mapping);
    void setMapping(int mapping, PlayerUID player, int dimension);

private:
    unsigned int mMappingWords[1280];
    unsigned char mDimensionMappings[32];
};

#endif
