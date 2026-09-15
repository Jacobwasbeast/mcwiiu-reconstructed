#include "game/world/block/Leaves2_BlockColor__L0__createDefault__11BlockColorsSFv.hpp"

#include "game/core/FoliageColor.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

int Leaves2_BlockColor__L0__createDefault__11BlockColorsSFv::getColor(const BlockState* state,
                                                                       LevelSource* levelSource,
                                                                       const BlockPos* pos,
                                                                       int tintIndex)
{
    if (levelSource != NULL && pos != NULL)
    {
        return BiomeColors::getAverageFoliageColor(levelSource, *pos);
    }
    return FoliageColor::getDefaultColor();
}
