#include "game/world/block/DoublePlant_BlockColor__L0__createDefault__11BlockColorsSFv.hpp"

#include "game/world/biome/BiomeColors.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DoublePlantBlock.hpp"

int DoublePlant_BlockColor__L0__createDefault__11BlockColorsSFv::getColor(
    const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
{
    DoublePlantBlock::Variant* variant = state->getValue(DoublePlantBlock::VARIANT);
    if (levelSource == NULL || pos == NULL ||
        (variant != gDoublePlantGrass && variant != gDoublePlantFern))
    {
        return -1;
    }

    BlockPos plantPos = state->getValue(DoublePlantBlock::HALF) ==
                                DoublePlantBlock::HALVES[DoublePlantBlock::Half::UPPER]
                            ? pos->below()
                            : *pos;
    return BiomeColors::getAverageGrassColor(levelSource, plantPos);
}
