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
        const int firstXOffsetStart = random.nextInt(8);
        const int firstXOffsetEnd = random.nextInt(8);
        const int firstYOffsetStart = random.nextInt(4);
        const int firstYOffsetEnd = random.nextInt(4);
        const int firstZOffsetStart = random.nextInt(8);
        const int firstZOffsetEnd = random.nextInt(8);
        const BlockPos first = origin.offset(firstXOffsetStart - firstXOffsetEnd,
                                             firstYOffsetStart - firstYOffsetEnd,
                                             firstZOffsetStart - firstZOffsetEnd);
        if (level->isEmptyBlock(first)
            && ((!level->mDimension->isHasCeiling()) || first.y < 126)
            && static_cast<DoublePlantBlock*>(Blocks::double_plant)->mayPlace(level, first))
        {
            static_cast<DoublePlantBlock*>(Blocks::double_plant)->placeAt(level, first, mNextPlantType, 2);
            placed = true;
        }

        const int secondXOffsetStart = random.nextInt(8);
        const int secondXOffsetEnd = random.nextInt(8);
        const int secondYOffsetStart = random.nextInt(4);
        const int secondYOffsetEnd = random.nextInt(4);
        const int secondZOffsetStart = random.nextInt(8);
        const int secondZOffsetEnd = random.nextInt(8);
        const BlockPos second = origin.offset(secondXOffsetStart - secondXOffsetEnd,
                                              secondYOffsetStart - secondYOffsetEnd,
                                              secondZOffsetStart - secondZOffsetEnd);
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
