#include "game/world/generation/SeagrassFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/TypedBoxed.hpp"

SeagrassFeature::SeagrassFeature()
    : Feature(false),
      mGenerating(false)
{
}

SeagrassFeature::~SeagrassFeature()
{
}

bool SeagrassFeature::isNotIce(Level* level, const BlockPos& pos)
{
    Block* block = level->getBlock(pos.below());
    return block != Blocks::ice && block != Blocks::packed_ice && block != Blocks::frosted_ice;
}

bool SeagrassFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (mGenerating && level->isFindingSpawn())
    {
        return false;
    }

    mGenerating = true;
    BlockPos position = origin;
    while (!level->isSolidBlockingBlock(position) && level->isWaterAt(position) && position.y > 0)
    {
        position = position.below();
    }

    if (!level->isWaterAt(position) || !level->isSolidBlockingBlock(position))
    {
        return false;
    }

    const BlockState* seagrassState = Blocks::sea_grass->defaultBlockState();
    if (random.nextInt(20) == 0)
    {
        for (int attempt = 0; attempt < 7; ++attempt)
        {
            BlockPos target = position.offset(random.nextGaussianInt(8), 0, random.nextGaussianInt(8));
            while (!level->isSolidBlockingBlock(target) && level->isWaterAt(target))
            {
                target = target.above();
            }

            if (level->isWaterAt(target)
                && level->isSolidBlockingBlock(target)
                && Blocks::sea_grass->canSurvive(level, target, seagrassState)
                && isNotIce(level, target))
            {
                level->setBlock(target, seagrassState, 0x12, false);
            }

            target = position.offset(random.nextGaussianInt(8), 0, random.nextGaussianInt(8));
            while (!level->isSolidBlockingBlock(target) && level->isWaterAt(target))
            {
                target = target.above();
            }

            if (level->isWaterAt(target)
                && level->isSolidBlockingBlock(target)
                && Blocks::sea_grass->canSurvive(level, target, seagrassState)
                && isNotIce(level, target))
            {
                level->setBlock(target, seagrassState, 0x12, false);
            }
        }
        mGenerating = false;
        return true;
    }

    if (random.nextInt(20) == 0)
    {
        const Property* property = Blocks::sea_grass->getBlockStateDefinition()->getProperties()[0];
        for (int attempt = 0; attempt < 8; ++attempt)
        {
            BlockPos target = position.offset(random.nextGaussianInt(8), 0, random.nextGaussianInt(8));
            while (!level->isSolidBlockingBlock(target) && level->isWaterAt(target))
            {
                target = target.above();
            }

            if (!(level->isWaterAt(target)
                && level->isSolidBlockingBlock(target)
                && Blocks::sea_grass->canSurvive(level, target, seagrassState)
                && isNotIce(level, target)))
            {
                continue;
            }

            if (!level->isSolidBlockingBlock(target.above(2)))
            {
                level->setBlock(target, seagrassState, 0x12, false);
            }
            else if (!level->isSolidBlockingBlock(target.above()) || random.nextInt(9) == 0)
            {
                level->setBlock(target, seagrassState, 0x12, false);
            }
            else
            {
                TypedBoxed<int> lowerValue(2);
                TypedBoxed<int> upperValue(1);
                const BlockState* lowerState = seagrassState->setBoxedValue(property, &lowerValue);
                const BlockState* upperState = seagrassState->setBoxedValue(property, &upperValue);
                level->setBlock(target, lowerState, 0x12, false);
                level->setBlock(target.above(), upperState, 0x12, false);
            }
        }
        mGenerating = false;
        return true;
    }

    for (int attempt = 0; attempt < 4; ++attempt)
    {
        BlockPos target = position.offset(random.nextGaussianInt(8), 0, random.nextGaussianInt(8));
        while (!level->isSolidBlockingBlock(target) && level->isWaterAt(target))
        {
            target = target.above();
        }

        if (level->isWaterAt(target)
            && level->isSolidBlockingBlock(target)
            && Blocks::sea_grass->canSurvive(level, target, seagrassState))
        {
            level->setBlock(target, seagrassState, 0x12, false);
        }

        target = position.offset(random.nextGaussianInt(8), 0, random.nextGaussianInt(8));
        while (!level->isSolidBlockingBlock(target) && level->isWaterAt(target))
        {
            target = target.above();
        }

        if (level->isWaterAt(target)
            && level->isSolidBlockingBlock(target)
            && Blocks::sea_grass->canSurvive(level, target, seagrassState))
        {
            level->setBlock(target, seagrassState, 0x12, false);
        }
    }

    mGenerating = false;
    return true;
}
