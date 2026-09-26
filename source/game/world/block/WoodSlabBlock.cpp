#include "game/world/block/Predicates.hpp"
#include "game/world/block/WoodSlabBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

EnumProperty<PlanksBlock::Variant*>* WoodSlabBlock::VARIANT = NULL;
static const int gWoodSlabDescriptionIds[6] = {
    (int)0xDE25DD5A, (int)0x74EF0A65, (int)0x26F5D1DD,
    (int)0xE0B76C20, (int)0x5C9C0D57, (int)0x1C5A6EC1
};

void WoodSlabBlock::blockStaticCtor()
{
    boost::unordered_set<PlanksBlock::Variant*> values;
    for (int i = 0; i < 6; ++i)
    {
        values.insert(PlanksBlock::Variant::byData(i));
    }

    VARIANT = EnumProperty<PlanksBlock::Variant*>::create(
        L"variant", typeid(PlanksBlock::Variant*), values, Predicates<PlanksBlock::Variant*>::alwaysTrue);
}

WoodSlabBlock::WoodSlabBlock()
    : HalfSlabBlock(Material::WOOD)
{
}

WoodSlabBlock::~WoodSlabBlock()
{
}

void WoodSlabBlock::DerivedInit()
{
    HalfSlabBlock::DerivedInit();

    const BlockState* state = mStateDefinition->any();
    if (!isFullSize())
    {
        TypedBoxed<Half*> half(BOTTOM);
        state = state->setBoxedValue(TYPE, &half);
    }

    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData(0));
    registerDefaultState(state->setBoxedValue(VARIANT, &variant));
}

const MaterialColor* WoodSlabBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(VARIANT)->getMaterialColor();
}

Item* WoodSlabBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::wooden_slab);
}

not_null_ptr<ItemInstance> WoodSlabBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wooden_slab, 1, data));
}

int WoodSlabBlock::getAuxName(int auxValue)
{
    return getDescriptionId(auxValue);
}

Property* WoodSlabBlock::getTypeProperty()
{
    return VARIANT;
}

Boxed* WoodSlabBlock::getTypeForItem(not_null_ptr<ItemInstance> itemInstance)
{
    return new TypedBoxed<PlanksBlock::Variant*>(
        PlanksBlock::Variant::byData(itemInstance->getAuxValue() & 7));
}

const BlockState* WoodSlabBlock::getBlockState(int data)
{
    TypedBoxed<PlanksBlock::Variant*> variant(
        PlanksBlock::Variant::byData(data & 7));
    const BlockState* state = defaultBlockState()->setBoxedValue(VARIANT, &variant);

    if (!isFullSize())
    {
        TypedBoxed<Half*> half((data & 8) == 0 ? BOTTOM : TOP);
        state = state->setBoxedValue(TYPE, &half);
    }

    return state;
}

int WoodSlabBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();

    if (!isFullSize() && state->getValue(TYPE) == TOP)
    {
        data |= 8;
    }

    return data;
}

BlockStateDefinition* WoodSlabBlock::createBlockStateDefinition()
{
    if (isFullSize())
    {
        const Property* properties[1] = { VARIANT };
        return new BlockStateDefinition(this, properties, 1);
    }

    const Property* properties[2] = { TYPE, VARIANT };
    return new BlockStateDefinition(this, properties, 2);
}

int WoodSlabBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

TextureAtlasSprite* WoodSlabBlock::getTexture(const Direction* direction, const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    return Blocks::planks->getTexture(direction, Blocks::planks->getBlockState(data));
}

void WoodSlabBlock::registerIcons(IconRegister*)
{
}

int WoodSlabBlock::getDescriptionId(int auxValue)
{
    const BlockState* state = getBlockState(-1);
    int data = state->getValue(VARIANT)->getData();
    if (data == -1)
    {
        data = 0;
    }
    return gWoodSlabDescriptionIds[data];
}
