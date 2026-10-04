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

    if (position.y > 3)
    {
        do
        {
            if (!level->isEmptyBlock(position.below()))
            {
                Block* blockBelow = level->getBlockState(position.below())->getBlock();
                if (blockBelow == Blocks::grass || blockBelow == Blocks::dirt || blockBelow == Blocks::stone)
                {
                    if (position.y > 3)
                    {
                        const int radiusBase = mRadius;
                        for (int layer = 0; radiusBase >= 0 && layer < 3; ++layer)
                        {
                            const int xRadius = radiusBase + random.nextInt(2);
                            const int yRadius = radiusBase + random.nextInt(2);
                            const int zRadius = radiusBase + random.nextInt(2);
                            const float radius = (float)(xRadius + yRadius + zRadius) * 0.333F + 0.5F;
                            const BlockPos from = position.offset(-xRadius, -yRadius, -zRadius);
                            const BlockPos to = position.offset(xRadius, yRadius, zRadius);
                            Iterator<BlockPos*>* positions = BlockPos::between(from, to);

                            while (positions->hasNext())
                            {
                                BlockPos candidate = *positions->next();
                                if (candidate.distSqr(position) <= (double)(radius * radius))
                                {
                                    level->setBlock(candidate, mBlock->defaultBlockState(), Block::UPDATE_INVISIBLE, false);
                                }
                            }

                            delete positions;
                            position = position.offset(random.nextInt(radiusBase * 2 + 2) - (radiusBase + 1),
                                                       -random.nextInt(2),
                                                       random.nextInt(radiusBase * 2 + 2) - (radiusBase + 1));
                        }

                        return true;
                    }

                    return false;
                }
            }

            position = position.below();
        }
        while (position.y > 3);
    }

    return false;
}
