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
#include "game/world/block/TypedBoxed.hpp"

static EnumProperty<PlanksBlock::Variant*>* gOldLogVariantProperty = NULL;
bool oldlog_filter__FPQ2_11PlanksBlock7Variant(PlanksBlock::Variant* value)
{
    return value != NULL && value->getData() < 4;
}

class OldLogVariantPredicate : public Predicate<PlanksBlock::Variant*>
{
public:
    virtual bool apply(PlanksBlock::Variant* value) const
    {
        return oldlog_filter__FPQ2_11PlanksBlock7Variant(value);
    }
};

EnumProperty<PlanksBlock::Variant*>* OldLogBlock::VARIANT = NULL;
Property* OldLogBlock_VARIANT = NULL;

void OldLogBlock::blockStaticCtor()
{
    boost::unordered_set<PlanksBlock::Variant*> values;
    values.insert(PlanksBlock::Variant::byData(0));
    values.insert(PlanksBlock::Variant::byData(1));
    values.insert(PlanksBlock::Variant::byData(2));
    values.insert(PlanksBlock::Variant::byData(3));
    values.insert(PlanksBlock::Variant::byData(4));
    values.insert(PlanksBlock::Variant::byData(5));
    OldLogVariantPredicate predicate;
    gOldLogVariantProperty = EnumProperty<PlanksBlock::Variant*>::create(
        L"variant", typeid(PlanksBlock::Variant*), values, &predicate);
    VARIANT = gOldLogVariantProperty;
    OldLogBlock_VARIANT = VARIANT;
}

OldLogBlock::OldLogBlock()
    : LogBlock()
{
    DerivedInit();
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData(0));
    const BlockState* state = RotatedPillarBlock::getBlockState(0);
    registerDefaultState(state->setBoxedValue(VARIANT, &variant));
}

OldLogBlock::~OldLogBlock()
{
}

const MaterialColor* OldLogBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    PlanksBlock::Variant* variant = state->getValue(VARIANT);
    if (getAxis(state)->getId() == Direction::Axis::eAxis_Y)
    {
        return variant->getMaterialColor();
    }
    int data = variant->getData();
    if (data == 1)
    {
        return PlanksBlock::Variant::byData(5)->getMaterialColor();
    }
    if (data != 2)
    {
        return PlanksBlock::Variant::byData(1)->getMaterialColor();
    }
    return MaterialColor::SAND;
}

const BlockState* OldLogBlock::getBlockState(int data)
{
    const BlockState* state = RotatedPillarBlock::getBlockState(data);
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData(data & 3));
    return state->setBoxedValue(VARIANT, &variant);
}

int OldLogBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    Direction::Axis* axis = getAxis(state);
    if (axis->getId() == Direction::Axis::eAxis_X)
    {
        data |= 4;
    }
    else if (axis->getId() == Direction::Axis::eAxis_Z)
    {
        data |= 8;
    }
    else if (axis->getId() != Direction::Axis::eAxis_Y)
    {
        data |= 12;
    }
    return data;
}

BlockStateDefinition* OldLogBlock::createBlockStateDefinition()
{
    BlockStateDefinition* axisDefinition = RotatedPillarBlock::createBlockStateDefinition();
    const std::vector<const Property*>& axisProperties = axisDefinition->getProperties();
    const Property* properties[2] = { VARIANT, axisProperties[0] };
    BlockStateDefinition* definition = new BlockStateDefinition(this, properties, 2);
    delete axisDefinition;
    return definition;
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

PlanksBlock::Variant* OldLogBlock::getWoodType(const BlockState* state)
{
    return state->getValue(VARIANT);
}
