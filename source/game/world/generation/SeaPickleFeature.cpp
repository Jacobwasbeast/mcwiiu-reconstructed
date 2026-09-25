#include "game/world/generation/SeaPickleFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

extern AbstractProperty<int>* SeaPickle_PICKLES;

SeaPickleFeature::~SeaPickleFeature()
{
}

bool SeaPickleFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (!level->mIsClientSide)
    {
        BlockPos position = origin;
        while (!level->isSolidBlockingBlock(position)
            && level->getBlockId(position.x, position.y, position.z) != 271
            && position.y > 0)
        {
            position = position.below();
        }

        if (level->isSolidBlockingBlock(position))
        {
            for (int attempt = 0; attempt < 4; ++attempt)
            {
                const int dz = random.nextInt(6);
                const int dx = random.nextInt(6);
                BlockPos target = position.offset(dx, 0, dz);
                const int pickles = random.nextInt(0, 4);
                while (!level->isEmptyWaterBlock(target)
                    && level->getBlockId(target.x, target.y, target.z) != 0)
                {
                    target = target.above();
                }

                if (level->getBlockId(target.x, target.y, target.z) != 0
                    && level->getBlockId(target.x, target.y, target.z) != 271
                    && Blocks::sea_pickle->canSurvive(level, target, Blocks::sea_pickle->defaultBlockState()))
                {
                    const BlockState* state = Blocks::sea_pickle->defaultBlockState();
                    TypedBoxed<int> value(pickles);
                    state = state->setBoxedValue(SeaPickle_PICKLES, &value);
                    setBlock(level, target, state, 0);
                }
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}
