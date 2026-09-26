#include "game/world/block/BlockBlobFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Iterator.hpp"

BlockBlobFeature::BlockBlobFeature(Block* block, int radius)
    : Feature(false), mBlock(block), mRadius(radius)
{
}

BlockBlobFeature::~BlockBlobFeature()
{
}

bool BlockBlobFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    BlockPos position = origin;

    while (level->isEmptyBlock(position.below()))
    {
        if (position.y < 4)
        {
            return false;
        }
        position = position.below();
    }

    if (position.y < 4)
    {
        return false;
    }

    Block* blockBelow = level->getBlockState(position.below())->getBlock();
    if (blockBelow != Blocks::stone && blockBelow != Blocks::grass && blockBelow != Blocks::dirt)
    {
        return false;
    }

    if (mRadius < 0)
    {
        return true;
    }

    for (int layer = 0; layer < 3; ++layer)
    {
        const int xRadius = mRadius + random.nextInt(2);
        const int yRadius = mRadius + random.nextInt(2);
        const int zRadius = mRadius + random.nextInt(2);
        const float radius = (float)(xRadius + yRadius + zRadius) * 0.333F + 0.5F;
        const BlockPos from = position.offset(-xRadius, -yRadius, -zRadius);
        const BlockPos to = position.offset(xRadius, yRadius, zRadius);
        Iterator<BlockPos*>* positions = BlockPos::between(from, to);

        while (positions->hasNext())
        {
            BlockPos* candidate = positions->next();
            if (candidate->distSqr(position) <= (double)(radius * radius))
            {
                level->setBlock(*candidate, mBlock->defaultBlockState(), Block::UPDATE_INVISIBLE, false);
            }
        }

        delete positions;
        position = position.offset(random.nextInt(mRadius * 2 + 2) - (mRadius + 1),
                                   -random.nextInt(2),
                                   random.nextInt(mRadius * 2 + 2) - (mRadius + 1));
    }

    return true;
}
