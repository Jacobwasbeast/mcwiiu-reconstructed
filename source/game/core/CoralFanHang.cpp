#include "game/core/CoralFanHang.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/CoralFan.hpp"
#include "game/core/Direction.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<int>* CoralFanHang::VARIANT = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"variantHang", 0, 1));
AbstractProperty<int>* CoralFanHang::DIRECTION = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"directionHang", 0, 3));
AbstractProperty<bool>* CoralFanHang::DEAD = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"dead_coral"));

namespace
{
    const AABB* CORAL_FAN_HANG_SHAPE = AABB::newPermanent(
        0.19999998807907104, 0.25, 0.19999998807907104,
        0.8f, 0.85f, 0.8f);
}

CoralFanHang::CoralFanHang()
    : CoralFan()
{
    ClearDerivedInit();
    DerivedInit();

    TypedBoxed<int> variant(0);
    TypedBoxed<int> facing(2);
    TypedBoxed<bool> dead(false);
    registerDefaultState(defaultBlockState()->setBoxedValue(CoralFanHang::VARIANT, &variant)
                                            ->setBoxedValue(CoralFanHang::DIRECTION, &facing)
                                            ->setBoxedValue(CoralFanHang::DEAD, &dead));
}

CoralFanHang::~CoralFanHang()
{
}

Item* CoralFanHang::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return NULL;
}

const AABB* CoralFanHang::getShape(const BlockState* state, LevelSource* levelSource,
                                   const BlockPos& pos)
{
    return CORAL_FAN_HANG_SHAPE;
}

const Direction* CoralFanHang::_getNeighborFromDirection(int direction) const
{
    if (direction == 0)
    {
        return Direction::WEST;
    }

    if (direction == 1)
    {
        return Direction::EAST;
    }

    if (direction == 2)
    {
        return Direction::NORTH;
    }

    return Direction::SOUTH;
}

void CoralFanHang::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                   Block* block, const BlockPos& fromPos)
{
    if (level->mIsClientSide)
    {
        return;
    }

    const Direction* facing =
        _getNeighborFromDirection(level->getBlockState(pos)->getValue(CoralFanHang::DIRECTION));
    BlockPos support = pos.relative(facing->getOpposite());

    if (!level->getBlockState(support)->canProvideSupport(facing, BLOCK_SUPPORT_CENTER))
    {
        level->addToTickNextTick(pos, this, 1);
        return;
    }

    level->addToTickNextTick(pos, this, level->mRandom->nextInt(40, 200));
}

TextureAtlasSprite* CoralFanHang::getTexture(const Direction* direction, const BlockState* state)
{
    int variant = state->getValue(CoralFanHang::VARIANT);

    if (this == Blocks::coral_fan_hang2)
    {
        variant += 2;
    }

    if (this == Blocks::coral_fan_hang3)
    {
        variant = 4;
    }

    if (state->getValue(CoralFanHang::DEAD))
    {
        return mDeadIcons[variant];
    }

    return mIcons[variant];
}

int CoralFanHang::convertBlockStateToLegacyData(const BlockState* state)
{
    return (state->getValue(CoralFanHang::VARIANT) << 1)
        | (state->getValue(CoralFanHang::DIRECTION) << 2)
        | (state->getValue(CoralFanHang::DEAD) ? 1 : 0);
}

const BlockState* CoralFanHang::getBlockState(int data)
{
    TypedBoxed<int> variant((data >> 1) & 1);
    TypedBoxed<int> facing((data >> 2) & 3);
    TypedBoxed<bool> dead((data & 1) != 0);

    return defaultBlockState()->setBoxedValue(CoralFanHang::VARIANT, &variant)
                              ->setBoxedValue(CoralFanHang::DIRECTION, &facing)
                              ->setBoxedValue(CoralFanHang::DEAD, &dead);
}

not_null_ptr<ItemInstance> CoralFanHang::getSilkTouchItemInstance(const BlockState* state)
{
    Block* fanBlock = state->getValue(CoralFanHang::DEAD) ? Blocks::coral_fan_dead
                                                         : Blocks::coral_fan;
    TypedBoxed<int> fanDead(0);
    const BlockState* fanState =
        fanBlock->defaultBlockState()->setBoxedValue(CoralFan::DIRECTION, &fanDead);

    if (state->getBlock() == Blocks::coral_fan_hang)
    {
        TypedBoxed<int> fanType(state->getValue(CoralFanHang::VARIANT));
        fanState = fanState->setBoxedValue(CoralFan::VARIANT, &fanType);
    }
    else if (state->getBlock() == Blocks::coral_fan_hang2)
    {
        TypedBoxed<int> fanType(state->getValue(CoralFanHang::VARIANT) == 0 ? 2 : 3);
        fanState = fanState->setBoxedValue(CoralFan::VARIANT, &fanType);
    }
    else
    {
        TypedBoxed<int> fanType(4);
        fanState = fanState->setBoxedValue(CoralFan::VARIANT, &fanType);
    }

    return not_null_ptr<ItemInstance>(
        new ItemInstance(fanState->getBlock(), 1,
                         Blocks::coral_fan->convertBlockStateToLegacyData(fanState)));
}

not_null_ptr<ItemInstance> CoralFanHang::getCloneItemInstance(Level* level, const BlockPos& pos,
                                                              const BlockState* state)
{
    Block* fanBlock = state->getValue(CoralFanHang::DEAD) ? Blocks::coral_fan_dead
                                                         : Blocks::coral_fan;
    TypedBoxed<int> fanDead(0);
    const BlockState* fanState =
        fanBlock->defaultBlockState()->setBoxedValue(CoralFan::DIRECTION, &fanDead);

    if (state->getBlock() == Blocks::coral_fan_hang)
    {
        TypedBoxed<int> fanType(state->getValue(CoralFanHang::VARIANT));
        fanState = fanState->setBoxedValue(CoralFan::VARIANT, &fanType);
    }
    else if (state->getBlock() == Blocks::coral_fan_hang2)
    {
        TypedBoxed<int> fanType(state->getValue(CoralFanHang::VARIANT) == 0 ? 2 : 3);
        fanState = fanState->setBoxedValue(CoralFan::VARIANT, &fanType);
    }
    else
    {
        TypedBoxed<int> fanType(4);
        fanState = fanState->setBoxedValue(CoralFan::VARIANT, &fanType);
    }

    return not_null_ptr<ItemInstance>(
        new ItemInstance(fanState->getBlock(), 1,
                         Blocks::coral_fan->convertBlockStateToLegacyData(fanState)));
}

BlockStateDefinition* CoralFanHang::createBlockStateDefinition()
{
    static const Property* PROPERTIES[3] =
    {
        CoralFanHang::VARIANT, CoralFanHang::DIRECTION, CoralFanHang::DEAD
    };

    return new BlockStateDefinition(this, PROPERTIES, 3);
}

void CoralFanHang::checkAlive(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* facing = _getNeighborFromDirection(state->getValue(CoralFanHang::DIRECTION));
    BlockPos support = pos.relative(facing->getOpposite());

    if (!level->isUnderWater(pos, Block::getId(this), 0))
    {
        TypedBoxed<bool> dead(true);
        level->setBlock(pos, state->setBoxedValue(CoralFanHang::DEAD, &dead), Block::UPDATE_ALL, false);
    }

    if (!level->isSolidBlockingBlock(support) && !level->getBlock(support)->isSlabBlock())
    {
        level->setBlockAndData(pos.x, pos.y, pos.z, BlockId::AIR, 0, Block::UPDATE_ALL);
    }
}

bool CoralFanHang::mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    const BlockState* support = level->getBlockState(pos);

    if (support->canProvideSupport(Direction::NORTH, BLOCK_SUPPORT_CENTER))
    {
        return true;
    }

    if (HalfSlabBlock::isHalfSlab(support))
    {
        return support->getValue(HalfSlabBlock::TYPE) == HalfSlabBlock::TOP;
    }

    return false;
}

void CoralFanHang::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!state->getValue(CoralFanHang::DEAD)
        && level->getLiquidBlock(pos)->mMaterial == mMaterial)
    {
        level->setExtraBlock(pos, Blocks::flowing_water->defaultBlockState(), Block::UPDATE_ALL, false);
    }
}

