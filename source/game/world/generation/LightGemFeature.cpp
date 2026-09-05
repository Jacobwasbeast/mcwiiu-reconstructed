#include "game/world/generation/LightGemFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

LightGemFeature::~LightGemFeature()
{
}

bool LightGemFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (!level->isEmptyBlock(origin))
    {
        return false;
    }

    if (level->getBlockState(origin.above())->getBlock() != Blocks::netherrack)
    {
        return false;
    }

    level->setBlock(origin, Blocks::glowstone->defaultBlockState(), 2, false);

    for (int i = 0; i < 750; ++i)
    {
        BlockPos position = origin.offset(random.nextInt(8) - random.nextInt(8),
                                          -random.nextInt(12),
                                          random.nextInt(8) - random.nextInt(8));
        if (level->getBlockState(position)->getBlock() == Blocks::AIR)
        {
            int glowstoneNeighbours = 0;
            std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
            while (it != Direction::VALUES.end())
            {
                if (level->getBlockState(position.relative(*it))->getBlock() == Blocks::glowstone)
                {
                    ++glowstoneNeighbours;
                    if (glowstoneNeighbours > 1)
                    {
                        break;
                    }
                }
                ++it;
            }

            if (glowstoneNeighbours == 1)
            {
                level->setBlock(position, Blocks::glowstone->defaultBlockState(), 2, false);
            }
        }

        position = origin.offset(random.nextInt(8) - random.nextInt(8),
                                 -random.nextInt(12),
                                 random.nextInt(8) - random.nextInt(8));
        if (level->getBlockState(position)->getBlock() == Blocks::AIR)
        {
            int glowstoneNeighbours = 0;
            std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
            while (it != Direction::VALUES.end())
            {
                if (level->getBlockState(position.relative(*it))->getBlock() == Blocks::glowstone)
                {
                    ++glowstoneNeighbours;
                    if (glowstoneNeighbours > 1)
                    {
                        break;
                    }
                }
                ++it;
            }

            if (glowstoneNeighbours == 1)
            {
                level->setBlock(position, Blocks::glowstone->defaultBlockState(), 2, false);
            }
        }
    }

    return true;
}
