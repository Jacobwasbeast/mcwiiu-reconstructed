#include "game/world/block/NewLogBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/core/Direction.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<PlanksBlock::Variant*>* NewLogBlock::VARIANT = NULL;

bool newleaf_predicate__FPQ2_11PlanksBlock7Variant(PlanksBlock::Variant* value)
{
    return value->getData() > 3;
}

void NewLogBlock::blockStaticCtor()
{
    boost::unordered_set<PlanksBlock::Variant*> values;
    values.insert(PlanksBlock::Variant::byData(0));
    values.insert(PlanksBlock::Variant::byData(1));
    values.insert(PlanksBlock::Variant::byData(2));
    values.insert(PlanksBlock::Variant::byData(3));
    values.insert(PlanksBlock::Variant::byData(4));
    values.insert(PlanksBlock::Variant::byData(5));
    VARIANT = EnumProperty<PlanksBlock::Variant*>::create(
        L"variant", typeid(PlanksBlock::Variant*), values,
        new Predicates<PlanksBlock::Variant*>::FunctionPredicate(
            newleaf_predicate__FPQ2_11PlanksBlock7Variant));
}

NewLogBlock::NewLogBlock()
    : LogBlock()
{
    DerivedInit();
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData(4));
    const BlockState* state = mStateDefinition->any()->setBoxedValue(VARIANT, &variant);
    TypedBoxed<LogAxis*> axis(Y);
    registerDefaultState(state->setBoxedValue(AXIS, &axis));
}

NewLogBlock::~NewLogBlock()
{
}

const MaterialColor* NewLogBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    PlanksBlock::Variant* variant = state->getValue(VARIANT);
    if (state->getValue(AXIS)->getId() == 1)
    {
        return variant->getMaterialColor();
    }
    if (variant->getData() == 5)
    {
        return variant->getMaterialColor();
    }
    return MaterialColor::STONE;
}

const BlockState* NewLogBlock::getBlockState(int data)
{
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData((data & 3) + 4));
    const BlockState* state = defaultBlockState()->setBoxedValue(VARIANT, &variant);
    LogAxis* axis = Y;
    if ((data & 0xC) == 4)
    {
        axis = X;
    }
    else if ((data & 0xC) == 8)
    {
        axis = Z;
    }
    else if ((data & 0xC) == 12)
    {
        axis = NONE;
    }
    TypedBoxed<LogAxis*> axisValue(axis);
    return state->setBoxedValue(AXIS, &axisValue);
}

int NewLogBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData() - 4;
    unsigned int axis = state->getValue(AXIS)->getId();
    if (axis == 0)
    {
        data |= 4;
    }
    else if (axis == 2)
    {
        data |= 8;
    }
    else if (axis == 3)
    {
        data |= 12;
    }
    return data;
}

BlockStateDefinition* NewLogBlock::createBlockStateDefinition()
{
    static const Property* properties[] = { VARIANT, AXIS };
    return new BlockStateDefinition(this, properties, 2);
}

not_null_ptr<ItemInstance> NewLogBlock::getSilkTouchItemInstance(const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1,
        getSpawnResourcesAuxValue(state)));
}

int NewLogBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData() - 4;
}

int NewLogBlock::getWoodType(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

bool NewLogBlock::isStrippable()
{
    return true;
}

const BlockState* NewLogBlock::getStrippedBlock(const BlockState* state)
{
    int woodType = getWoodType(state);
    const BlockState* strippedState;
    if (woodType == 4)
    {
        strippedState = Blocks::stripped_log_acacia->defaultBlockState();
    }
    else if (woodType == 5)
    {
        strippedState = Blocks::stripped_log_dark_oak->defaultBlockState();
    }
    else
    {
        return state;
    }
    if (strippedState == NULL)
    {
        return state;
    }
    BlockStateDefinition* axisDefinition = RotatedPillarBlock::createBlockStateDefinition();
    const Property* axisProperty = axisDefinition->getProperties()[0];
    unsigned int axisId = state->getValue(AXIS)->getId();
    if (axisId == 3)
    {
        delete axisDefinition;
        return strippedState;
    }
    Direction::Axis* directionAxis = axisId == 0 ? Direction::Axis::X :
        axisId == 1 ? Direction::Axis::Y : Direction::Axis::Z;
    TypedBoxed<Direction::Axis*> axis(directionAxis);
    const BlockState* result = strippedState->setBoxedValue(axisProperty, &axis);
    delete axisDefinition;
    return result;
}
