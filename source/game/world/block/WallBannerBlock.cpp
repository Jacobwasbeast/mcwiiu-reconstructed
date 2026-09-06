#include "game/world/block/WallBannerBlock.hpp"

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
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

static const AABB* WALL_BANNER_SHAPE_NORTH = AABB::newPermanent(0.0, 0.0, 0.75, 1.0, 1.0, 1.0);
static const AABB* WALL_BANNER_SHAPE_SOUTH = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.25);
static const AABB* WALL_BANNER_SHAPE_WEST = AABB::newPermanent(0.75, 0.0, 0.0, 1.0, 1.0, 1.0);
static const AABB* WALL_BANNER_SHAPE_EAST = AABB::newPermanent(0.0, 0.0, 0.0, 0.25, 1.0, 1.0);

WallBannerBlock::WallBannerBlock()
    : BannerBlock()
{
    DerivedInit();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(DirectionalBlock::FACING, &facing));
}

WallBannerBlock::~WallBannerBlock()
{
}

const BlockState* WallBannerBlock::rotate(const BlockState* state, Rotation* rotation)
{
    const Direction* direction = state->getValue(DirectionalBlock::FACING);
    TypedBoxed<const Direction*> facing(rotation->rotate(direction));
    return state->setBoxedValue(DirectionalBlock::FACING, &facing);
}

const BlockState* WallBannerBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(DirectionalBlock::FACING)));
}

const AABB* WallBannerBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    const Direction* direction = state->getValue(DirectionalBlock::FACING);
    if (direction == Direction::SOUTH)
        return WALL_BANNER_SHAPE_SOUTH;
    if (direction == Direction::WEST)
        return WALL_BANNER_SHAPE_WEST;
    if (direction == Direction::EAST)
        return WALL_BANNER_SHAPE_EAST;
    return WALL_BANNER_SHAPE_NORTH;
}

void WallBannerBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                      Block* block, const BlockPos& fromPos)
{
    const Direction* direction = state->getValue(DirectionalBlock::FACING);
    const BlockPos supportPos = pos.relative(direction->getOpposite());
    if (!level->getBlockState(supportPos)->getMaterial()->isSolid())
    {
        spawnResources(level, pos, state, 0.0f, 0);
        level->removeBlock(pos);
    }
    Block::neighborChanged(state, level, pos, block, fromPos);
}

const BlockState* WallBannerBlock::getBlockState(int data)
{
    const Direction* direction = Direction::from3DDataValue(data);
    if (direction->getAxis() == Direction::Axis::Y)
        direction = Direction::NORTH;
    TypedBoxed<const Direction*> facing(direction);
    return defaultBlockState()->setBoxedValue(DirectionalBlock::FACING, &facing);
}

int WallBannerBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(DirectionalBlock::FACING)->get3DDataValue();
}

BlockStateDefinition* WallBannerBlock::createBlockStateDefinition()
{
    static const Property* properties[] = { DirectionalBlock::FACING };
    return new BlockStateDefinition(this, properties, 1);
}
