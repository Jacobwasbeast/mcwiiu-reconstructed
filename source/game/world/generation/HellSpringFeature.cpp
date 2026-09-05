#include "game/world/generation/HellSpringFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

HellSpringFeature::HellSpringFeature(Block* block, bool requiresBlock)
    : Feature(false), mBlock(block), mRequiresBlock(requiresBlock)
{
}

HellSpringFeature::~HellSpringFeature()
{
}

bool HellSpringFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (level->getBlockState(origin.above())->getBlock() != Blocks::netherrack)
    {
        return false;
    }

    const BlockState* originState = level->getBlockState(origin);
    Block* originBlock = originState->getBlock();
    if (originBlock->mFullBlock && originBlock != Blocks::netherrack)
    {
        return false;
    }

    int netherrackNeighbours = 0;
    int emptyNeighbours = 0;
    const BlockPos neighbours[5] =
    {
        origin.west(), origin.east(), origin.north(), origin.south(), origin.below()
    };

    for (int i = 0; i < 5; ++i)
    {
        if (level->getBlockState(neighbours[i])->getBlock() == Blocks::netherrack)
        {
            ++netherrackNeighbours;
        }
        if (level->isEmptyBlock(neighbours[i]))
        {
            ++emptyNeighbours;
        }
    }

    if (!mRequiresBlock && netherrackNeighbours == 4)
    {
        if (emptyNeighbours != 1)
        {
            return false;
        }
    }
    else if (netherrackNeighbours != 5)
    {
        return false;
    }

    const BlockState* state = mBlock->defaultBlockState();
    level->setBlock(origin, state, 2, false);
    level->instaTick(origin, state, random);
    return true;
}
