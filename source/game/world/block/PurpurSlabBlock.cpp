#include "game/world/block/PurpurSlabBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Boxed.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

static PurpurSlabBlock::Variant* gPurpurSlabDefaultVariant = new PurpurSlabBlock::Variant(L"default");
static PurpurSlabBlock::Variant* gPurpurSlabVariants[1] = { gPurpurSlabDefaultVariant };
static EnumProperty<PurpurSlabBlock::Variant*>* gPurpurSlabVariantProperty = NULL;

PurpurSlabBlock::Variant::Variant(std::wstring name)
    : mName(name)
{
}

std::wstring PurpurSlabBlock::Variant::toString() const
{
    return mName;
}

std::wstring PurpurSlabBlock::Variant::getSerializedName() const
{
    return mName;
}

void PurpurSlabBlock::blockStaticCtor()
{
    boost::unordered_set<Variant*> values;
    values.insert(gPurpurSlabVariants[0]);
    gPurpurSlabVariantProperty = EnumProperty<Variant*>::create(
        L"variant", typeid(Variant*), values, Predicates<Variant*>::alwaysTrue);
}

PurpurSlabBlock::PurpurSlabBlock()
    : HalfSlabBlock(Material::STONE, MaterialColor::COLOR_MAGENTA)
{
}

void PurpurSlabBlock::DerivedInit()
{
    HalfSlabBlock::DerivedInit();
    const BlockState* state = mStateDefinition->any();
    if (!isFullSize())
    {
        TypedBoxed<Half*> half(BOTTOM);
        state = state->setBoxedValue(TYPE, &half);
    }
    TypedBoxed<Variant*> variant(gPurpurSlabDefaultVariant);
    registerDefaultState(state->setBoxedValue(gPurpurSlabVariantProperty, &variant));
}

Item* PurpurSlabBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::purpur_slab);
}

not_null_ptr<ItemInstance> PurpurSlabBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::purpur_slab));
}

const BlockState* PurpurSlabBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> variant(gPurpurSlabDefaultVariant);
    const BlockState* state = defaultBlockState()->setBoxedValue(gPurpurSlabVariantProperty, &variant);
    if (!isFullSize())
    {
        TypedBoxed<Half*> half((data & 8) == 0 ? BOTTOM : TOP);
        state = state->setBoxedValue(TYPE, &half);
    }
    return state;
}

int PurpurSlabBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    if (!isFullSize() && state->getValue(HalfSlabBlock::TYPE) == HalfSlabBlock::TOP)
    {
        return 8;
    }
    return 0;
}

int PurpurSlabBlock::getAuxName(int)
{
    return getDescriptionId(-1);
}

Property* PurpurSlabBlock::getTypeProperty()
{
    return gPurpurSlabVariantProperty;
}

Boxed* PurpurSlabBlock::getTypeForItem(not_null_ptr<ItemInstance>)
{
    return new TypedBoxed<Variant*>(gPurpurSlabDefaultVariant);
}

BlockStateDefinition* PurpurSlabBlock::createBlockStateDefinition()
{
    if (isFullSize())
    {
        static const Property* properties[1] = { gPurpurSlabVariantProperty };
        return new BlockStateDefinition(this, properties, 1);
    }

    static const Property* properties[2] = { HalfSlabBlock::TYPE, gPurpurSlabVariantProperty };
    return new BlockStateDefinition(this, properties, 2);
}
