#include "game/world/generation/DeadBushFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

DeadBushFeature::DeadBushFeature(Block* block)
    : Feature(false)
{
}

DeadBushFeature::~DeadBushFeature()
{
}

bool DeadBushFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    BlockPos position = origin;
    while ((level->getBlockState(position)->getMaterial() == Material::AIR
            || level->getBlockState(position)->getMaterial() == Material::PLANT)
           && position.y >= 1)
    {
        position = position.below();
    }

    for (int attempt = 0; attempt < 2; ++attempt)
    {
        const BlockPos target = position.offset(random.nextInt(8) - random.nextInt(8),
                                                random.nextInt(4) - random.nextInt(4),
                                                random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(target) && Blocks::deadbush->mayPlace(level, target))
        {
            level->setBlock(target, Blocks::deadbush->defaultBlockState(), 2, false);
        }
    }

    return true;
}
