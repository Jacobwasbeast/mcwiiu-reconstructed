#include "game/world/generation/VinesFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"

class VineBlock
{
public:
    static const BooleanProperty* NORTH;
    static const BooleanProperty* EAST;
    static const BooleanProperty* SOUTH;
    static const BooleanProperty* WEST;
};

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
            int xOffset = random.nextInt(4);
            int zOffset = random.nextInt(4);
            int xSubtraction = random.nextInt(4);
            int zSubtraction = random.nextInt(4);
            position = position.offset(xOffset - xSubtraction, 0,
                                       zOffset - zSubtraction);
        }
        else
        {
            arrayWithLength<const Direction*> directions =
                Direction::Plane::HORIZONTAL->iterator();
            bool movedUp = false;
            for (unsigned int i = 0; i < directions.length; ++i)
            {
                const Direction* direction = directions.data[i];
                if (level->isEmptyBlock(position.relative(direction)))
                {
                    TypedBoxed<bool> north(direction == Direction::NORTH);
                    TypedBoxed<bool> east(direction == Direction::EAST);
                    TypedBoxed<bool> south(direction == Direction::SOUTH);
                    TypedBoxed<bool> west(direction == Direction::WEST);
                    const BlockState* state = Blocks::vine->defaultBlockState();
                    state = state->setBoxedValue(VineBlock::NORTH, &north);
                    state = state->setBoxedValue(VineBlock::EAST, &east);
                    state = state->setBoxedValue(VineBlock::SOUTH, &south);
                    state = state->setBoxedValue(VineBlock::WEST, &west);
                    level->setBlock(position, state, 2, false);
                    position = position.above();
                    movedUp = true;
                    break;
                }
            }

            if (movedUp)
            {
                continue;
            }
        }

        position = position.above();
    }

    return true;
}
