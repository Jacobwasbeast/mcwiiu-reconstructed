#include "game/world/generation/BlueIceFeature.hpp"
#include "game/world/block/BlockId.hpp"

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
        && (level->getBlockId(origin.x, origin.y, origin.z) == BlockId::WATER
            || level->getBlockId(origin.x, origin.y - 1, origin.z) == BlockId::WATER))
    {
        for (std::vector<const Direction*>::iterator direction = Direction::VALUES.begin();
             direction != Direction::VALUES.end(); ++direction)
        {
            if (*direction != Direction::DOWN
                && level->getBlockState(origin.relative(*direction))->getBlock() == Blocks::packed_ice)
            {
                level->setBlock(origin, Blocks::blue_ice->defaultBlockState(), Block::UPDATE_CLIENTS, false);

                for (int i = 0; i < 200; ++i)
                {
                    const int verticalOffsetStart = random.nextInt(5);
                    const int verticalOffsetEnd = random.nextInt(6);
                    const int verticalOffset = verticalOffsetStart - verticalOffsetEnd;
                    int spread = 3;
                    if (verticalOffset <= 1)
                    {
                        spread = verticalOffset / 2 + 3;
                    }

                    if (spread > 0)
                    {
                        const int xOffsetStart = random.nextInt(spread);
                        const int xOffsetEnd = random.nextInt(spread);
                        const int zOffsetStart = random.nextInt(spread);
                        const int zOffsetEnd = random.nextInt(spread);
                        const BlockPos position = origin.offset(
                            xOffsetStart - xOffsetEnd,
                            verticalOffset,
                            zOffsetStart - zOffsetEnd);
                        const BlockState* state = level->getBlockState(position);
                        Block* block = state->getBlock();
                        if (state->getMaterial() == Material::AIR
                            || block == Blocks::water
                            || block == Blocks::packed_ice
                            || block == Blocks::ice)
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

                            if (blueIceCount > 0 && blueIceCount <= 4)
                            {
                                level->setBlock(position, Blocks::blue_ice->defaultBlockState(), Block::UPDATE_CLIENTS, false);
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
