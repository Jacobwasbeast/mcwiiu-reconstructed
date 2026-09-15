#include "game/world/block/Leaves1_BlockColor__L0__createDefault__11BlockColorsSFv.hpp"

#include "game/core/FoliageColor.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"

int Leaves1_BlockColor__L0__createDefault__11BlockColorsSFv::getColor(const BlockState* state,
                                                                       LevelSource* levelSource,
                                                                       const BlockPos* pos,
                                                                       int tintIndex)
{
    PlanksBlock::Variant* variant = state->getValue(OldLeafBlock::VARIANT);
    if (variant == PlanksBlock::Variant::SPRUCE)
    {
        return FoliageColor::getEvergreenColor();
    }
    if (variant == PlanksBlock::Variant::BIRCH)
    {
        return FoliageColor::getBirchColor();
    }
    if (levelSource != NULL && pos != NULL)
    {
        return BiomeColors::getAverageFoliageColor(levelSource, *pos);
    }
    return FoliageColor::getDefaultColor();
}
