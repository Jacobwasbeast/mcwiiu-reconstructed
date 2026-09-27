#include "game/world/generation/VinesFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/VineBlock.hpp"

VinesFeature::VinesFeature()
    : Feature(false)
{
}

VinesFeature::~VinesFeature()
{
}

bool VinesFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    BlockPos position(origin);

    while (position.y <= 127)
    {
        if (!level->isEmptyBlock(position))
        {
            const int xOffsetStart = random.nextInt(4);
            const int xOffsetEnd = random.nextInt(4);
            const int zOffsetStart = random.nextInt(4);
            const int zOffsetEnd = random.nextInt(4);
            position = position.offset(xOffsetStart - xOffsetEnd, 0, zOffsetStart - zOffsetEnd);
        }
        else
        {
            arrayWithLength<const Direction*> directions =
                Direction::Plane::HORIZONTAL->iterator();
            bool placed = false;
            for (unsigned int i = 0; i < directions.length; ++i)
            {
                const Direction* direction = directions.data[i];
                if (Blocks::vine->mayPlace(level, position, direction))
                {
                    const BlockState* state = Blocks::vine->defaultBlockState();
                    TypedBoxed<bool> north(direction == Direction::SOUTH);
                    state = state->setBoxedValue(VineBlock::NORTH, &north);
                    TypedBoxed<bool> east(direction == Direction::WEST);
                    state = state->setBoxedValue(VineBlock::EAST, &east);
                    TypedBoxed<bool> south(direction == Direction::NORTH);
                    state = state->setBoxedValue(VineBlock::SOUTH, &south);
                    TypedBoxed<bool> west(direction == Direction::EAST);
                    state = state->setBoxedValue(VineBlock::WEST, &west);
                    level->setBlock(position, state, Block::UPDATE_CLIENTS, false);
                    position = position.above();
                    placed = true;
                    break;
                }
            }

            if (placed)
            {
                continue;
            }
        }

        position = position.above();
    }

    return true;
}
