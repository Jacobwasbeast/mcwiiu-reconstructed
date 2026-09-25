#include "game/world/generation/KelpFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/TypedBoxed.hpp"

KelpFeature::KelpFeature()
    : Feature(false)
{
    const Property* ageProperty = Blocks::kelp->getBlockStateDefinition()->getProperties()[0];
    int age;
    for (age = 0; age < 16; ++age)
    {
        TypedBoxed<int> value(age);
        mStates[age] = Blocks::kelp->defaultBlockState()->setBoxedValue(ageProperty, &value);
    }
}

KelpFeature::~KelpFeature()
{
}

bool KelpFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (!level->mIsClientSide)
    {
        BlockPos position(origin);
        while (!level->isSolidBlockingBlock(position)
            && level->getBlockId(position.x, position.y, position.z) != 258
            && position.y > 0)
        {
            position = position.below();
        }

        if (level->isSolidBlockingBlock(position))
        {
            int attempt;
            for (attempt = 0; attempt < 10; ++attempt)
            {
                const int dz = random.nextGaussianInt(8);
                const int dx = random.nextGaussianInt(8);
                BlockPos candidate = position.offset(dx, 0, dz);
                while (!level->isEmptyWaterBlock(candidate)
                    && level->getBlockId(candidate.x, candidate.y, candidate.z) != 0)
                {
                    candidate = candidate.above();
                }

                if (level->getBlockId(candidate.x, candidate.y, candidate.z) != 0
                    && !level->isEmptyWaterBlock(candidate.below())
                    && level->getBlockId(candidate.x, candidate.y, candidate.z) != 258)
                {
                    int height = 0;
                    while (level->isEmptyWaterBlock(candidate.above(height)))
                    {
                        ++height;
                        if (height >= 16)
                        {
                            break;
                        }
                    }

                    if (height >= 3)
                    {
                        int growth = random.nextInt(3, height);
                        if (Blocks::kelp->mayPlace(level, candidate))
                        {
                            int y;
                            setBlock(level, candidate, mStates[15 - growth], 0x30);
                            for (y = 1; y < growth; ++y)
                            {
                                BlockPos plantPosition = candidate.above(y);
                                if (!level->isEmptyWaterBlock(plantPosition)
                                    || !Blocks::kelp->mayPlace(level, plantPosition))
                                {
                                    break;
                                }
                                setBlock(level, plantPosition, mStates[15 - growth + y], 0x30);
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool KelpFeature::_canPlace(Level* level, const BlockPos& pos) const
{
    int blockId = level->getBlockId(pos.x, pos.y, pos.z);
    return blockId == 3 || blockId == 12 || blockId == 13 || blockId == 258;
}
