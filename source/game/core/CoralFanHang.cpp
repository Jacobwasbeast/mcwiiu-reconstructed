#include "game/core/CoralFanHang.hpp"

#include "game/core/Direction.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

static AbstractProperty<int>* CORAL_FAN_HANG_VARIANT = NULL;
static AbstractProperty<int>* CORAL_FAN_HANG_FACING = NULL;
static AbstractProperty<bool>* CORAL_FAN_HANG_DEAD = NULL;

Property* CoralFanHang_PROPERTY_1 = NULL;
Property* CoralFanHang_PROPERTY_2 = NULL;
Property* CoralFanHang_PROPERTY_3 = NULL;

CoralFanHang::CoralFanHang()
    : CoralFan()
{
    ClearDerivedInit();
    DerivedInit();

    TypedBoxed<int> variant(0);
    TypedBoxed<int> facing(2);
    TypedBoxed<bool> dead(false);
    const BlockState* state = defaultBlockState();
    state = state->setBoxedValue(CORAL_FAN_HANG_VARIANT, &variant);
    state = state->setBoxedValue(CORAL_FAN_HANG_FACING, &facing);
    state = state->setBoxedValue(CORAL_FAN_HANG_DEAD, &dead);
    registerDefaultState(state);
}

CoralFanHang::~CoralFanHang()
{
}

Item* CoralFanHang::getResource(const BlockState*, Random*, int)
{
    return NULL;
}

const AABB* CoralFanHang::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

const Direction* CoralFanHang::_getNeighborFromDirection(int direction) const
{
    if (direction == 0)
        return Direction::NORTH;
    if (direction == 1)
        return Direction::SOUTH;
    if (direction == 2)
        return Direction::WEST;
    return Direction::EAST;
}

void CoralFanHang::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                   Block*, const BlockPos&)
{
    if (!level->mIsClientSide)
    {
        const Direction* direction = _getNeighborFromDirection(state->getValue(CORAL_FAN_HANG_FACING));
        BlockPos support = pos.relative(direction->getOpposite());
        if (!level->getBlockState(support)->canProvideSupport(direction, BLOCK_SUPPORT_CENTER))
            level->levelEvent(2001, pos, 1);
        else
            level->levelEvent(level->mRandom->nextInt(40, 200), pos, 1);
    }
}

TextureAtlasSprite* CoralFanHang::getTexture(const Direction* direction, const BlockState* state)
{
    return CoralFan::getTexture(direction, state);
}

int CoralFanHang::convertBlockStateToLegacyData(const BlockState* state)
{
    return (state->getValue(CORAL_FAN_HANG_VARIANT) << 1)
        | (state->getValue(CORAL_FAN_HANG_FACING) << 2)
        | (state->getValue(CORAL_FAN_HANG_DEAD) ? 1 : 0);
}

const BlockState* CoralFanHang::getBlockState(int data)
{
    TypedBoxed<int> variant((data >> 1) & 1);
    TypedBoxed<int> facing((data >> 2) & 3);
    TypedBoxed<bool> dead((data & 1) != 0);
    const BlockState* state = defaultBlockState()->setBoxedValue(CORAL_FAN_HANG_VARIANT, &variant);
    state = state->setBoxedValue(CORAL_FAN_HANG_FACING, &facing);
    return state->setBoxedValue(CORAL_FAN_HANG_DEAD, &dead);
}

not_null_ptr<ItemInstance> CoralFanHang::getSilkTouchItemInstance(const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(this, 1, convertBlockStateToLegacyData(state)));
}

not_null_ptr<ItemInstance> CoralFanHang::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(this, 1, convertBlockStateToLegacyData(state)));
}

BlockStateDefinition* CoralFanHang::createBlockStateDefinition()
{
    if (CORAL_FAN_HANG_VARIANT == NULL)
    {
        CORAL_FAN_HANG_VARIANT = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"variant", 0, 1));
        CORAL_FAN_HANG_FACING = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"facing", 0, 3));
        CORAL_FAN_HANG_DEAD = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"dead"));
        CoralFanHang_PROPERTY_1 = CORAL_FAN_HANG_VARIANT;
        CoralFanHang_PROPERTY_2 = CORAL_FAN_HANG_FACING;
        CoralFanHang_PROPERTY_3 = CORAL_FAN_HANG_DEAD;
    }
    const Property* properties[3] =
    {
        CORAL_FAN_HANG_VARIANT, CORAL_FAN_HANG_FACING, CORAL_FAN_HANG_DEAD
    };
    return new BlockStateDefinition(this, properties, 3);
}

void CoralFanHang::checkAlive(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* direction = _getNeighborFromDirection(state->getValue(CORAL_FAN_HANG_FACING));
    BlockPos support = pos.relative(direction->getOpposite());
    if (!level->isUnderWater(pos, Block::getId(this), 0))
    {
        TypedBoxed<bool> dead(true);
        level->setBlock(pos, state->setBoxedValue(CORAL_FAN_HANG_DEAD, &dead), 3, false);
    }
    if (!level->isSolidBlockingBlock(support)
        && !level->getBlockState(support)->canProvideSupport(direction, BLOCK_SUPPORT_CENTER))
        level->setBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
}

bool CoralFanHang::mayPlaceOn(Level* level, const BlockPos& pos, const BlockState*)
{
    const BlockState* support = level->getBlockState(pos);
    if (support->canProvideSupport(Direction::DOWN, BLOCK_SUPPORT_CENTER))
        return true;
    if (HalfSlabBlock::isHalfSlab(support))
        return support->getValue(HalfSlabBlock::TYPE) == HalfSlabBlock::TOP;
    return false;
}

void CoralFanHang::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!state->getValue(CORAL_FAN_HANG_DEAD)
        && level->getExtraBlock(pos)->mBaseItemType == mBaseItemType)
        level->setExtraBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
}
