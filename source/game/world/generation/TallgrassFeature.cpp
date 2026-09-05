#include "game/world/generation/TallgrassFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"

TallgrassFeature::TallgrassFeature(TallGrassBlock::Variant* variant)
    : Feature(false)
{
    TypedBoxed<TallGrassBlock::Variant*> value(variant);
    mState = Blocks::tallgrass->defaultBlockState()->setBoxedValue(TallGrassBlock::VARIANT, &value);
}

TallgrassFeature::~TallgrassFeature()
{
}

bool TallgrassFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    BlockPos position(origin);

    while (level->getBlockState(position)->getBlock() == Blocks::AIR ||
           level->getBlockState(position)->getBlock() == Blocks::WATER)
    {
        if (position.y < 1)
        {
            break;
        }
        position = position.below();
    }

    for (int i = 0; i < 64; ++i)
    {
        BlockPos candidate = position.offset(random.nextInt(8) - random.nextInt(8),
                                              random.nextInt(4) - random.nextInt(4),
                                              random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(candidate) &&
            Blocks::tallgrass->canSurvive(level, candidate, mState))
        {
            level->setBlock(candidate, mState, 18, false);
        }

        candidate = position.offset(random.nextInt(8) - random.nextInt(8),
                                    random.nextInt(4) - random.nextInt(4),
                                    random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(candidate) &&
            Blocks::tallgrass->canSurvive(level, candidate, mState))
        {
            level->setBlock(candidate, mState, 18, false);
        }
    }

    return true;
}
