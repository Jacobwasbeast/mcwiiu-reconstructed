#include "game/world/block/PoweredRailBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<BaseRailBlock::RailShape*>* PoweredRailBlock::SHAPE = NULL;
BooleanProperty* PoweredRailBlock::POWERED = static_cast<BooleanProperty*>(BooleanProperty::create(L"powered"));

static const BlockState* poweredRailState(const BlockState* state, const Property* property,
                                          BaseRailBlock::RailShape* shape, bool powered)
{
    TypedBoxed<BaseRailBlock::RailShape*> shapeValue(shape);
    const BlockState* result = state->setBoxedValue(property, &shapeValue);
    TypedBoxed<bool> poweredValue(powered);
    return result->setBoxedValue(PoweredRailBlock::POWERED, &poweredValue);
}

void PoweredRailBlock::blockStaticCtor()
{
    boost::unordered_set<BaseRailBlock::RailShape*> values;
    for (int i = 0; i < 10; ++i)
        values.insert(RailShape::VALUES[i]);
    PoweredRailBlock::SHAPE = EnumProperty<BaseRailBlock::RailShape*>::create(
        L"shape", typeid(BaseRailBlock::RailShape*), values,
        new Predicates<BaseRailBlock::RailShape*>::FunctionPredicate(railfilter));
}

PoweredRailBlock::PoweredRailBlock()
    : BaseRailBlock(true), mPoweredTexture(NULL)
{
    DerivedInit();
    TypedBoxed<BaseRailBlock::RailShape*> shapeValue(RailShape::NORTH_SOUTH);
    TypedBoxed<bool> poweredValue(false);
    const BlockState* state = mStateDefinition->any()->setBoxedValue(PoweredRailBlock::SHAPE, &shapeValue);
    registerDefaultState(state->setBoxedValue(PoweredRailBlock::POWERED, &poweredValue));
}

PoweredRailBlock::~PoweredRailBlock()
{
}

bool PoweredRailBlock::findPoweredRailSignal(Level* level, const BlockPos& pos,
                                              const BlockState* state, bool forward, int distance)
{
    if (distance > 7)
        return false;

    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    bool checkBelow = true;
    BaseRailBlock::RailShape* shape = state->getValue(PoweredRailBlock::SHAPE);
    switch (shape->getData())
    {
    case 0:
        if (forward)
            ++z;
        else
            --z;
        break;
    case 1:
        if (forward)
            --x;
        else
            ++x;
        break;
    case 2:
        if (forward)
        {
            --x;
        }
        else
        {
            ++x;
            ++y;
            checkBelow = false;
        }
        shape = RailShape::EAST_WEST;
        break;
    case 3:
        if (forward)
        {
            --x;
            ++y;
            checkBelow = false;
        }
        else
        {
            ++x;
        }
        shape = RailShape::EAST_WEST;
        break;
    case 4:
        if (forward)
        {
            ++z;
        }
        else
        {
            --z;
            ++y;
            checkBelow = false;
        }
        shape = RailShape::NORTH_SOUTH;
        break;
    case 5:
        if (forward)
        {
            ++z;
            ++y;
            checkBelow = false;
        }
        else
        {
            --z;
        }
        shape = RailShape::NORTH_SOUTH;
        break;
    }

    if (isSameRailWithPower(level, BlockPos(x, y, z), forward, distance, shape))
        return true;
    return checkBelow && isSameRailWithPower(level, BlockPos(x, y - 1, z), forward, distance, shape);
}

bool PoweredRailBlock::isSameRailWithPower(Level* level, const BlockPos& pos, bool forward,
                                           int distance, BaseRailBlock::RailShape* shape)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() != this)
        return false;
    BaseRailBlock::RailShape* otherShape = state->getValue(PoweredRailBlock::SHAPE);
    if (shape == RailShape::EAST_WEST &&
        (otherShape == RailShape::NORTH_SOUTH || otherShape == RailShape::ASCENDING_NORTH ||
         otherShape == RailShape::ASCENDING_SOUTH))
        return false;
    if (shape == RailShape::NORTH_SOUTH &&
        (otherShape == RailShape::EAST_WEST || otherShape == RailShape::ASCENDING_EAST ||
         otherShape == RailShape::ASCENDING_WEST))
        return false;
    if (!state->getValue(PoweredRailBlock::POWERED))
        return false;
    if (level->hasNeighborSignal(pos))
        return true;
    return findPoweredRailSignal(level, pos, state, forward, distance + 1);
}

void PoweredRailBlock::updateState(const BlockState* state, Level* level, const BlockPos& pos, Block*)
{
    bool wasPowered = state->getValue(PoweredRailBlock::POWERED);
    bool powered = level->hasNeighborSignal(pos) || findPoweredRailSignal(level, pos, state, true, 0)
        || findPoweredRailSignal(level, pos, state, false, 0);
    if (powered != wasPowered)
    {
        TypedBoxed<bool> poweredValue(powered);
        const BlockState* updated = state->setBoxedValue(PoweredRailBlock::POWERED, &poweredValue);
        level->setBlock(pos, updated, Block::UPDATE_ALL, false);
        level->updateNeighborsAt(pos.below(), this, false);
        if (state->getValue(PoweredRailBlock::SHAPE)->isAscending())
            level->updateNeighborsAt(pos.above(), this, false);
    }
}

const AbstractProperty<BaseRailBlock::RailShape*>* PoweredRailBlock::getShapeProperty()
{
    return PoweredRailBlock::SHAPE;
}

const BlockState* PoweredRailBlock::getBlockState(int data)
{
    return poweredRailState(defaultBlockState(), PoweredRailBlock::SHAPE,
                            BaseRailBlock::RailShape::byData(data & 7), (data & 8) != 0);
}

int PoweredRailBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(PoweredRailBlock::SHAPE)->getData();
    if (state->getValue(PoweredRailBlock::POWERED))
        data |= 8;
    return data;
}

const BlockState* PoweredRailBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
        switch (state->getValue(PoweredRailBlock::SHAPE)->getData())
        {
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_EAST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_SOUTH);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_NORTH);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        }
    case Rotation::eTYPE_CounterClockwise90:
        switch (state->getValue(PoweredRailBlock::SHAPE)->getData())
        {
        case 0:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::EAST_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 1:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_SOUTH);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_NORTH);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_SOUTH);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_EAST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        }
    case Rotation::eTYPE_Clockwise90:
        switch (state->getValue(PoweredRailBlock::SHAPE)->getData())
        {
        case 0:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::EAST_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 1:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_SOUTH);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_SOUTH);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_NORTH);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_EAST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        }
    default:
        break;
    }

    return state;
}

const BlockState* PoweredRailBlock::mirror(const BlockState* state, Mirror* mirror)
{
    if (mirror->getType() == Mirror::eTYPE_LeftRight)
    {
        switch (state->getValue(PoweredRailBlock::SHAPE)->getData())
        {
        case 4:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_SOUTH);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 5:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_NORTH);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        }
    }
    else if (mirror->getType() == Mirror::eTYPE_FrontBack)
    {
        switch (state->getValue(PoweredRailBlock::SHAPE)->getData())
        {
        case 2:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 3:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::ASCENDING_EAST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 6:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 7:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::SOUTH_EAST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 8:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_EAST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        case 9:
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(RailShape::NORTH_WEST);
            return state->setBoxedValue(PoweredRailBlock::SHAPE, &shape);
        }
        }
    }

    return Block::mirror(state, mirror);
}

BlockStateDefinition* PoweredRailBlock::createBlockStateDefinition()
{
    const Property* properties[2] = { PoweredRailBlock::SHAPE, PoweredRailBlock::POWERED };
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* PoweredRailBlock::getTexture(const Direction*, const BlockState* state)
{
    return state->getValue(PoweredRailBlock::POWERED) ? mPoweredTexture : mIcon;
}

void PoweredRailBlock::registerIcons(IconRegister* iconRegister)
{
    Block::registerIcons(iconRegister);
    mPoweredTexture = iconRegister->registerIcon(getIconName() + L"_powered");
}
