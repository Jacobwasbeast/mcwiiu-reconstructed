#include "game/world/block/WallSignBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

DirectionProperty* WallSignBlock::FACING = NULL;

static const AABB* WALL_SIGN_SHAPE_EAST = AABB::newPermanent(0.0, 0.25, 0.0, 0.125, 0.75, 1.0);
static const AABB* WALL_SIGN_SHAPE_WEST = AABB::newPermanent(0.875, 0.25, 0.0, 1.0, 0.75, 1.0);
static const AABB* WALL_SIGN_SHAPE_SOUTH = AABB::newPermanent(0.0, 0.25, 0.0, 1.0, 0.75, 0.125);
static const AABB* WALL_SIGN_SHAPE_NORTH = AABB::newPermanent(0.0, 0.25, 0.875, 1.0, 0.75, 1.0);

WallSignBlock::WallSignBlock()
    : SignBlock()
{
    DerivedInit();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(FACING, &facing));
}

WallSignBlock::~WallSignBlock()
{
}

void WallSignBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
}

const AABB* WallSignBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    switch (state->getValue(FACING)->get3DDataValue())
    {
    case 3: return WALL_SIGN_SHAPE_SOUTH;
    case 4: return WALL_SIGN_SHAPE_NORTH;
    case 5: return WALL_SIGN_SHAPE_EAST;
    default: return WALL_SIGN_SHAPE_NORTH;
    }
}

void WallSignBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                    Block* block, const BlockPos& fromPos)
{
    const Direction* facing = state->getValue(FACING);
    const BlockState* support = level->getBlockState(pos.relative(facing->getOpposite()));
    if (!support->isSolid())
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
    Block::neighborChanged(state, level, pos, block, fromPos);
}

const BlockState* WallSignBlock::getBlockState(int data)
{
    const Direction* facing = Direction::from3DDataValue(data);
    if (facing->getAxis() == Direction::Axis::Y)
        facing = Direction::NORTH;
    TypedBoxed<const Direction*> boxedFacing(facing);
    return defaultBlockState()->setBoxedValue(FACING, &boxedFacing);
}

int WallSignBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(FACING)->get3DDataValue();
}

const BlockState* WallSignBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}

const BlockState* WallSignBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}

BlockStateDefinition* WallSignBlock::createBlockStateDefinition()
{
    static const Property* properties[1];
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;
        properties[0] = FACING;
    }
    return new BlockStateDefinition(this, properties, 1);
}
