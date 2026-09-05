#include "game/world/generation/BlueIceFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

BlueIceFeature::~BlueIceFeature()
{
}

bool BlueIceFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (origin.y <= level->getSeaLevel() - 1
        && (level->getBlockState(origin)->getMaterial() == Material::WATER
            || level->getBlockState(origin.below())->getMaterial() == Material::WATER))
    {
        for (std::vector<const Direction*>::iterator direction = Direction::VALUES.begin();
             direction != Direction::VALUES.end(); ++direction)
        {
            if (*direction != Direction::DOWN
                && level->getBlockState(origin.relative(*direction))->getBlock() == Blocks::packed_ice)
            {
                level->setBlock(origin, Blocks::blue_ice->defaultBlockState(), 2, false);

                for (int i = 0; i < 200; ++i)
                {
                    const int verticalOffset =
                        random.nextInt(5) - random.nextInt(6);
                    int spread = 3;
                    if (1 < verticalOffset)
                    {
                        spread = verticalOffset / 2 + 3;
                    }

                    if (spread > 0)
                    {
                        const BlockPos position = origin.offset(
                            random.nextInt(spread) - random.nextInt(spread),
                            verticalOffset,
                            random.nextInt(spread) - random.nextInt(spread));
                        const BlockState* state = level->getBlockState(position);
                        if (state->getMaterial() == Material::WATER
                            || state->getBlock() == Blocks::ice
                            || state->getBlock() == Blocks::packed_ice
                            || state->getBlock() == Blocks::blue_ice)
                        {
                            int blueIceCount = 0;
                            for (std::vector<const Direction*>::iterator adjacent = Direction::VALUES.begin();
                                 adjacent != Direction::VALUES.end(); ++adjacent)
                            {
                                if (level->getBlockState(position.relative(*adjacent))->getBlock()
                                    == Blocks::blue_ice)
                                {
                                    ++blueIceCount;
                                    if (blueIceCount > 4)
                                    {
                                        break;
                                    }
                                }
                            }

                            if (blueIceCount <= 4)
                            {
                                level->setBlock(position, Blocks::blue_ice->defaultBlockState(), 2, false);
                            }
                        }
                    }
                }

                return true;
            }
        }
    }

    return false;
}
