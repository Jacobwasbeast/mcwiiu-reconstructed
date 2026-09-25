#include "game/world/generation/HellFireFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

HellFireFeature::~HellFireFeature()
{
}

bool HellFireFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    for (int i = 0; i < 64; ++i)
    {
        const int dx = random.nextInt(8) - random.nextInt(8);
        const int dy = random.nextInt(4) - random.nextInt(4);
        const int dz = random.nextInt(8) - random.nextInt(8);
        const BlockPos position = origin.offset(dx, dy, dz);
        if (level->isEmptyBlock(position))
        {
            if (level->getBlockState(position.below())->getBlock() == Blocks::netherrack)
            {
                level->setBlock(position, Blocks::FIRE->defaultBlockState(), 2, false);
            }
        }
    }

    return true;
}
