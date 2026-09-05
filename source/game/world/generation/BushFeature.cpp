#include "game/world/generation/BushFeature.hpp"

#include "game/core/Bush.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

BushFeature::BushFeature(Bush* bush)
    : Feature(false), mBush(bush)
{
}

BushFeature::~BushFeature()
{
}

bool BushFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    for (int attempt = 0; attempt < 32; ++attempt)
    {
        const BlockPos first = origin.offset(random.nextInt(8) - random.nextInt(8),
                                             random.nextInt(4) - random.nextInt(4),
                                             random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(first)
            && (!level->mDimension->isHasCeiling() || first.y < 127)
            && mBush->canSurvive(level, first, mBush->defaultBlockState()))
        {
            level->setBlock(first, mBush->defaultBlockState(), 2, false);
        }

        const BlockPos second = origin.offset(random.nextInt(8) - random.nextInt(8),
                                              random.nextInt(4) - random.nextInt(4),
                                              random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(second))
        {
            if (level->mDimension->isHasCeiling() && second.y >= 127)
            {
                continue;
            }
            if (mBush->canSurvive(level, second, mBush->defaultBlockState()))
            {
                level->setBlock(second, mBush->defaultBlockState(), 2, false);
            }
        }
    }

    return true;
}
