#include "game/world/generation/SeagrassFeature.hpp"

#include "game/core/SeaGrass.hpp"
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
    while (!level->isSolidBlockingBlock(position) && position.y > 0)
    {
        position = position.below();
    }

    if (!level->isSolidBlockingBlock(position))
    {
        return false;
    }

    const BlockState* seagrassState = Blocks::sea_grass->defaultBlockState();
    if (random.nextInt(20) == 0)
    {
        for (int attempt = 0; attempt < 7; ++attempt)
        {
            const int dz = random.nextGaussianInt(8);
            const int dx = random.nextGaussianInt(8);
            BlockPos target = position.offset(dx, 0, dz);
            while (!level->isEmptyWaterBlock(target)
                && level->getBlockId(target.x, target.y, target.z) != 0)
            {
                target = target.above();
            }

            if (level->getBlockId(target.x, target.y, target.z) != 0
                && level->isEmptyWaterBlock(target)
                && static_cast<SeaGrass*>(Blocks::sea_grass)->canSurvive(level, target, seagrassState, true)
                && isNotIce(level, target))
            {
                level->setBlock(target, seagrassState, 0x12, false);
            }

            const int dz2 = random.nextGaussianInt(8);
            const int dx2 = random.nextGaussianInt(8);
            target = position.offset(dx2, 0, dz2);
            while (!level->isEmptyWaterBlock(target)
                && level->getBlockId(target.x, target.y, target.z) != 0)
            {
                target = target.above();
            }

            if (level->getBlockId(target.x, target.y, target.z) != 0
                && level->isEmptyWaterBlock(target)
                && static_cast<SeaGrass*>(Blocks::sea_grass)->canSurvive(level, target, seagrassState, true)
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
            const int dz = random.nextGaussianInt(8);
            const int dx = random.nextGaussianInt(8);
            BlockPos target = position.offset(dx, 0, dz);
            while (!level->isEmptyWaterBlock(target)
                && level->getBlockId(target.x, target.y, target.z) != 0)
            {
                target = target.above();
            }

            if (!(level->getBlockId(target.x, target.y, target.z) != 0
                && level->isEmptyWaterBlock(target)
                && static_cast<SeaGrass*>(Blocks::sea_grass)->canSurvive(level, target, seagrassState, true)
                && isNotIce(level, target)))
            {
                continue;
            }

            if (!level->isEmptyWaterBlock(target.above(2)))
            {
                level->setBlock(target, seagrassState, 0x12, false);
            }
            else if (!level->isEmptyWaterBlock(target.above()) || random.nextInt(9) == 0)
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
        const int dz = random.nextGaussianInt(8);
        const int dx = random.nextGaussianInt(8);
        BlockPos target = position.offset(dx, 0, dz);
        while (!level->isEmptyWaterBlock(target)
            && level->getBlockId(target.x, target.y, target.z) != 0)
        {
            target = target.above();
        }

        if (level->getBlockId(target.x, target.y, target.z) != 0
            && level->isEmptyWaterBlock(target)
            && static_cast<SeaGrass*>(Blocks::sea_grass)->canSurvive(level, target, seagrassState, true))
        {
            level->setBlock(target, seagrassState, 0x12, false);
        }

        const int dz2 = random.nextGaussianInt(8);
        const int dx2 = random.nextGaussianInt(8);
        target = position.offset(dx2, 0, dz2);
        while (!level->isEmptyWaterBlock(target)
            && level->getBlockId(target.x, target.y, target.z) != 0)
        {
            target = target.above();
        }

        if (level->getBlockId(target.x, target.y, target.z) != 0
            && level->isEmptyWaterBlock(target)
            && static_cast<SeaGrass*>(Blocks::sea_grass)->canSurvive(level, target, seagrassState, true))
        {
            level->setBlock(target, seagrassState, 0x12, false);
        }
    }

    mGenerating = false;
    return true;
}
