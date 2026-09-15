#include "game/world/block/Grass_BlockColor__L0__createDefault__11BlockColorsSFv.hpp"

#include "game/core/GrassColor.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/block/BlockPos.hpp"

int Grass_BlockColor__L0__createDefault__11BlockColorsSFv::getColor(
    const BlockState*, LevelSource* levelSource, const BlockPos* pos, int)
{
    if (levelSource != NULL && pos != NULL)
        return (int)BiomeColors::getAverageGrassColor(levelSource, *pos);

    return (int)GrassColor::get(0.5, 1.0);
}
