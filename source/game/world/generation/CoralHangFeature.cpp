#include "game/world/generation/CoralHangFeature.hpp"

#include "game/core/CoralFanHang.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"


CoralHangFeature::CoralHangFeature()
    : Feature(false)
{
}

CoralHangFeature::~CoralHangFeature()
{
}

bool CoralHangFeature::canPlaceHangOn(Level* level, const BlockPos& pos)
{
    Block* block = level->getBlock(pos);
    return block == Blocks::coral_block || block == Blocks::stone;
}

const Direction* CoralHangFeature::randomDirection(Level* level, Random& random,
                                                    const BlockPos& pos)
{
    arrayWithLength<const Direction*> faces = Direction::Plane::HORIZONTAL->getFaces();
    while (faces.length != 0)
    {
        const unsigned int index = (unsigned int)random.nextInt(0, (int)faces.length);
        const Direction* direction = faces.data[index];
        if (level->isEmptyWaterBlock(pos.relative(direction)))
        {
            return direction;
        }

        faces.data[index] = faces.data[faces.length - 1];
        --faces.length;
        faces.data[faces.length] = direction;
    }

    return Direction::NORTH;
}

int CoralHangFeature::turnFacingIntoint(const Direction* direction)
{
    if (direction == Direction::NORTH)
    {
        return 2;
    }
    if (direction == Direction::EAST)
    {
        return 1;
    }
    if (direction != Direction::SOUTH)
    {
        return 0;
    }
    return 3;
}

bool CoralHangFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (level->mIsClientSide)
    {
        return true;
    }

    BlockPos position = origin;
    while (!level->isSolidBlockingBlock(position) && position.y > 50)
    {
        position = position.below();
    }

    if (level->isSolidBlockingBlock(position) && canPlaceHangOn(level, position))
    {
        const Direction* direction = randomDirection(level, random, position);
        const BlockPos target = position.relative(direction);
        if (level->isEmptyWaterBlock(target)
            && level->isUnderWater(target, 266, 0))
        {
            const BlockState* state = Blocks::coral_fan_hang->defaultBlockState();
            TypedBoxed<int> variant(random.nextInt(CoralFanHang::VARIANT->getValueCount()));
            state = state->setBoxedValue(CoralFanHang::VARIANT, &variant);
            TypedBoxed<bool> dead(random.nextBoolean());
            state = state->setBoxedValue(CoralFanHang::DEAD, &dead);
            TypedBoxed<int> facing(turnFacingIntoint(direction));
            state = state->setBoxedValue(CoralFanHang::DIRECTION, &facing);
            if (!level->reallyHasChunkAt(target))
            {
                return true;
            }
            setBlock(level, target, state, Block::UPDATE_NONE);
            return true;
        }
    }

    return false;
}
