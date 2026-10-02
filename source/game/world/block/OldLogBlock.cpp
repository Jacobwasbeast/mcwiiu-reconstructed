#include "game/world/block/OldLogBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

bool oldlog_filter(PlanksBlock::Variant* value)
{
    return value->getData() < 4;
}

EnumProperty<PlanksBlock::Variant*>* OldLogBlock::VARIANT = NULL;

void OldLogBlock::blockStaticCtor()
{
    std::wstring name(L"variant");
    Predicates<PlanksBlock::Variant*>::FunctionPredicate* filter =
        new Predicates<PlanksBlock::Variant*>::FunctionPredicate(oldlog_filter);
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

OldLogBlock::OldLogBlock()
    : LogBlock()
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(VARIANT, PlanksBlock::Variant::OAK)->setValue(AXIS, Y));
}

const MaterialColor* OldLogBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    PlanksBlock::Variant* variant = state->getValue(VARIANT);
    if (state->getValue(AXIS)->getId() == 1)
    {
        return variant->getMaterialColor();
    }
    int data = variant->getData();
    if (data == 1)
    {
        return PlanksBlock::Variant::DARK_OAK->getMaterialColor();
    }
    if (data != 2)
    {
        return PlanksBlock::Variant::SPRUCE->getMaterialColor();
    }
    return MaterialColor::QUARTZ;
}

const BlockState* OldLogBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState()->setValue(VARIANT, PlanksBlock::Variant::byData(data & 3));
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

int OldLogBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
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

BlockStateDefinition* OldLogBlock::createBlockStateDefinition()
{
    static const Property* properties[] = { VARIANT, AXIS };
    return new BlockStateDefinition(this, properties, 2);
}

not_null_ptr<ItemInstance> OldLogBlock::getSilkTouchItemInstance(const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1,
        state->getValue(VARIANT)->getData()));
}

int OldLogBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

int OldLogBlock::getWoodType(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}
