#include "game/world/generation/PumpkinFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

PumpkinFeature::~PumpkinFeature()
{
}

bool PumpkinFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    for (int i = 0; i < 64; ++i)
    {
        const int xOffsetStart = random.nextInt(8);
        const int dx = xOffsetStart - random.nextInt(8);
        const int yOffsetStart = random.nextInt(4);
        const int dy = yOffsetStart - random.nextInt(4);
        const int zOffsetStart = random.nextInt(8);
        const int dz = zOffsetStart - random.nextInt(8);
        const BlockPos position = origin.offset(dx, dy, dz);
        if (level->isEmptyBlock(position)
            && level->getBlockState(position.below())->getBlock() == Blocks::grass
            && Blocks::pumpkin->mayPlace(level, position))
        {
            const BlockState* state = Blocks::pumpkin->defaultBlockState();
            TypedBoxed<const Direction*> facing(
                Direction::Plane::HORIZONTAL->getRandomDirection(random));
            level->setBlock(position,
                            state->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing),
                            Block::UPDATE_CLIENTS, false);
        }
    }

    return true;
}
