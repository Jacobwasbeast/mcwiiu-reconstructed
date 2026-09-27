#include "game/world/generation/EndIslandFeature.hpp"

#include <cmath>

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

EndIslandFeature::~EndIslandFeature()
{
}

bool EndIslandFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    float radius = (float)random.nextInt(3) + 4.0f;
    int y = 0;

    while (radius > 0.5f)
    {
        for (int x = (int)floorf(-radius); x <= (int)ceilf(radius); ++x)
        {
            for (int z = (int)floorf(-radius); z <= (int)ceilf(radius); ++z)
            {
                if ((float)(x * x + z * z) <= (radius + 1.0f) * (radius + 1.0f))
                {
                    const BlockPos position = origin.offset(x, y, z);
                    setBlock(level, position, Blocks::end_stone->defaultBlockState(), Block::UPDATE_NONE);
                }
            }
        }

        radius -= (float)random.nextInt(2) + 0.5f;
        --y;
    }

    return true;
}
