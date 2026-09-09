#include "game/world/generation/DoublePlantFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"

DoublePlantFeature* gDoublePlantFeature = new DoublePlantFeature();

DoublePlantFeature::DoublePlantFeature()
    : Feature(false)
{
}

DoublePlantFeature::~DoublePlantFeature()
{
}

void DoublePlantFeature::setNextPlantType(DoublePlantBlock::Variant* plantType)
{
    mNextPlantType = plantType;
}

bool DoublePlantFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    bool placed = false;

    for (int attempt = 0; attempt < 32; ++attempt)
    {
        const BlockPos first = origin.offset(random.nextInt(8) - random.nextInt(8),
                                             random.nextInt(4) - random.nextInt(4),
                                             random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(first)
            && ((!level->mDimension->isHasCeiling()) || first.y < 126)
            && static_cast<DoublePlantBlock*>(Blocks::double_plant)->mayPlace(level, first))
        {
            static_cast<DoublePlantBlock*>(Blocks::double_plant)->placeAt(level, first, mNextPlantType, 2);
            placed = true;
        }

        const BlockPos second = origin.offset(random.nextInt(8) - random.nextInt(8),
                                              random.nextInt(4) - random.nextInt(4),
                                              random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(second)
            && ((!level->mDimension->isHasCeiling()) || second.y < 126)
            && static_cast<DoublePlantBlock*>(Blocks::double_plant)->mayPlace(level, second))
        {
            static_cast<DoublePlantBlock*>(Blocks::double_plant)->placeAt(level, second, mNextPlantType, 2);
            placed = true;
        }
    }

    return placed;
}
