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
    for (int attempt = 0; attempt < 10; ++attempt)
    {
        const int xOffsetStart = random.nextInt(8);
        const int x = origin.x + xOffsetStart - random.nextInt(8);
        const int yOffsetStart = random.nextInt(4);
        const int y = origin.y + yOffsetStart - random.nextInt(4);
        const int zOffsetStart = random.nextInt(8);
        const int z = origin.z + zOffsetStart - random.nextInt(8);
        if (level->isEmptyBlock(BlockPos(x, y, z))
            && Blocks::waterlily->mayPlace(level, BlockPos(x, y, z)))
        {
            level->setBlock(BlockPos(x, y, z), Blocks::waterlily->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        }
    }

    return true;
}
