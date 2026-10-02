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

bool newleaf_predicate(PlanksBlock::Variant* value)
{
    return value->getData() >= 4;
}

void NewLogBlock::blockStaticCtor()
{
    std::wstring name(L"variant");
    Predicates<PlanksBlock::Variant*>::FunctionPredicate* filter =
        new Predicates<PlanksBlock::Variant*>::FunctionPredicate(newleaf_predicate);
    EnumProperty<PlanksBlock::Variant*>* variant;
    {
        boost::unordered_set<PlanksBlock::Variant*> values;
        values.insert(PlanksBlock::Variant::OAK);
        values.insert(PlanksBlock::Variant::SPRUCE);
        values.insert(PlanksBlock::Variant::BIRCH);
        values.insert(PlanksBlock::Variant::JUNGLE);
        values.insert(PlanksBlock::Variant::ACACIA);
        values.insert(PlanksBlock::Variant::DARK_OAK);
        variant = EnumProperty<PlanksBlock::Variant*>::create(name, typeid(PlanksBlock::Variant*), values, filter);
    }
    VARIANT = variant;
}

NewLogBlock::NewLogBlock()
    : LogBlock()
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(VARIANT, PlanksBlock::Variant::ACACIA)->setValue(AXIS, Y));
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
        return PlanksBlock::Variant::DARK_OAK->getMaterialColor();
    }
    return MaterialColor::STONE;
}

const BlockState* NewLogBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState()->setValue(VARIANT, PlanksBlock::Variant::byData((data & 3) + 4));
    switch (data & 0xC)
    {
    case 0:
        return state->setValue(AXIS, Y);
    case 4:
        return state->setValue(AXIS, X);
    case 8:
        return state->setValue(AXIS, Z);
    default:
        return state->setValue(AXIS, NONE);
    }
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
        state->getValue(VARIANT)->getData() - 4));
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
    const BlockState* stripped;
    int woodType = getWoodType(state);
    if (woodType == 4)
    {
        stripped = Blocks::stripped_log_acacia->defaultBlockState();
    }
    else
    {
        if (woodType != 5)
        {
            return state;
        }
        stripped = Blocks::stripped_log_dark_oak->defaultBlockState();
    }
    if (stripped == NULL)
    {
        return state;
    }
    unsigned int axisId = state->getValue(AXIS)->getId();
    if (axisId == 0)
    {
        stripped = stripped->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::X);
    }
    else if (axisId == 1)
    {
        stripped = stripped->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::Y);
    }
    else if (axisId == 2)
    {
        stripped = stripped->setValue(gRotatedPillarBlockAxisProperty, Direction::Axis::Z);
    }
    return stripped;
}
