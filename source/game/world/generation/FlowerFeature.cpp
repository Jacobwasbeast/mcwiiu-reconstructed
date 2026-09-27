#include "game/world/generation/FlowerFeature.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/TypedBoxed.hpp"

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
    if (g_pApp->getLevelGenerationOptions() != NULL
        && g_pApp->getLevelGenerationOptions()->checkIntersects(origin.offset(-8, -4, -8), origin.offset(8, 4, 8)))
    {
        return false;
    }

    for (int attempt = 0; attempt < 64; ++attempt)
    {
        const int xOffsetStart = random.nextInt(8);
        const int dx = xOffsetStart - random.nextInt(8);
        const int yOffsetStart = random.nextInt(4);
        const int dy = yOffsetStart - random.nextInt(4);
        const int zOffsetStart = random.nextInt(8);
        const int dz = zOffsetStart - random.nextInt(8);
        const BlockPos position = origin.offset(dx, dy, dz);
        if (level->isEmptyBlock(position)
            && (!level->mDimension->isHasCeiling() || position.y < 127)
            && mFlowerBlock->canSurvive(level, position, mState))
        {
            level->setBlock(position, mState, Block::UPDATE_CLIENTS, false);
        }
    }

    return true;
}
