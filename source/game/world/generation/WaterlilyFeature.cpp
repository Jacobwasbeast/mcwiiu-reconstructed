#include "game/world/generation/WaterlilyFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

WaterlilyFeature::~WaterlilyFeature()
{
}

bool WaterlilyFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    for (int attempt = 0; attempt < 5; ++attempt)
    {
        const BlockPos position = origin.offset(random.nextInt(8) - random.nextInt(8),
                                                 random.nextInt(4) - random.nextInt(4),
                                                 random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(position)
            && Blocks::waterlily->mayPlace(level, position))
        {
            level->setBlock(position, Blocks::waterlily->defaultBlockState(), 2, false);
        }
    }

    return true;
}
