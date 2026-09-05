#include "game/world/generation/FlowerFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/TypedBoxed.hpp"

class CMinecraftApp
{
public:
    LevelGenerationOptions* getLevelGenerationOptions();
};

class LevelGenerationOptions
{
public:
    bool checkIntersects(const BlockPos& min, const BlockPos& max);
};

extern CMinecraftApp* g_pApp;

FlowerFeature::FlowerFeature(FlowerBlock* block, FlowerBlock::Flower* flower)
    : Feature(false)
{
    setNextFlower(block, flower);
}

FlowerFeature::~FlowerFeature()
{
}

void FlowerFeature::setNextFlower(FlowerBlock* block, FlowerBlock::Flower* flower)
{
    mFlowerBlock = block;
    TypedBoxed<FlowerBlock::Flower*> value(flower);
    mState = mFlowerBlock->defaultBlockState()->setBoxedValue(mFlowerBlock->getFlowerProperty(), &value);
}

bool FlowerFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (options != NULL && options->checkIntersects(origin.offset(-8, -4, -8), origin.offset(8, 4, 8)))
    {
        return false;
    }

    for (int attempt = 0; attempt < 32; ++attempt)
    {
        BlockPos position = origin.offset(random.nextInt(8) - random.nextInt(8),
                                          random.nextInt(4) - random.nextInt(4),
                                          random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(position)
            && (!level->mDimension->isHasCeiling() || position.y < 127)
            && mFlowerBlock->canSurvive(level, position, mState))
        {
            level->setBlock(position, mState, 2, false);
        }

        position = origin.offset(random.nextInt(8) - random.nextInt(8),
                                 random.nextInt(4) - random.nextInt(4),
                                 random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(position))
        {
            if (level->mDimension->isHasCeiling() && position.y >= 127)
            {
                continue;
            }
            if (mFlowerBlock->canSurvive(level, position, mState))
            {
                level->setBlock(position, mState, 2, false);
            }
        }
    }

    return true;
}
