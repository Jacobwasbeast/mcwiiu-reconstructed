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
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<PlanksBlock::Variant*>* NewLogBlock::VARIANT = NULL;

class NewLogVariantPredicate : public Predicate<PlanksBlock::Variant*>
{
public:
    virtual bool apply(PlanksBlock::Variant* value) const
    {
        return value != NULL && value->getData() >= 4;
    }
};

void NewLogBlock::blockStaticCtor()
{
    boost::unordered_set<PlanksBlock::Variant*> values;
    values.insert(PlanksBlock::Variant::byData(4));
    values.insert(PlanksBlock::Variant::byData(5));
    NewLogVariantPredicate predicate;
    VARIANT = EnumProperty<PlanksBlock::Variant*>::create(
        L"variant", typeid(PlanksBlock::Variant*), values, &predicate);
}

NewLogBlock::NewLogBlock()
    : LogBlock()
{
    DerivedInit();
    BlockStateDefinition* axisDefinition = LogBlock::createBlockStateDefinition();
    const Property* axisProperty = axisDefinition->getProperties()[0];
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData(4));
    TypedBoxed<LogBlock::LogAxis*> axis(LogBlock::LogAxis::fromAxis(Direction::Axis::Y));
    const BlockState* state = mStateDefinition->any()->setBoxedValue(axisProperty, &axis);
    registerDefaultState(state->setBoxedValue(VARIANT, &variant));
    delete axisDefinition;
}

NewLogBlock::~NewLogBlock()
{
}

const MaterialColor* NewLogBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    PlanksBlock::Variant* variant = state->getValue(VARIANT);
    if (variant->getData() == 5)
    {
        return variant->getMaterialColor();
    }
    return MaterialColor::NONE;
}

const BlockState* NewLogBlock::getBlockState(int data)
{
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData((data & 3) + 4));
    return defaultBlockState()->setBoxedValue(VARIANT, &variant);
}

int NewLogBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData() - 4;
}

BlockStateDefinition* NewLogBlock::createBlockStateDefinition()
{
    BlockStateDefinition* axisDefinition = LogBlock::createBlockStateDefinition();
    const Property* properties[] = { axisDefinition->getProperties()[0], VARIANT };
    BlockStateDefinition* definition = new BlockStateDefinition(this, properties, 2);
    delete axisDefinition;
    return definition;
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
    Block* stripped = woodType == 4 ? Blocks::stripped_log_acacia :
        woodType == 5 ? Blocks::stripped_log_dark_oak : NULL;
    if (stripped == NULL)
    {
        return state;
    }
    return stripped->defaultBlockState();
}
