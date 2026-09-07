#include "game/world/block/StoneSlabBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/Direction.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

static StoneSlabBlock::Variant* gStoneSlabVariants[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
static EnumProperty<StoneSlabBlock::Variant*>* gStoneSlabVariantProperty = NULL;
static Property* gStoneSlabSeamlessProperty = BooleanProperty::create(L"seamless");

StoneSlabBlock::Variant::Variant(int data, const MaterialColor* materialColor, const std::wstring& name, int translationKey)
    : mData(data), mName(name), mTranslationKey(translationKey), mColor(materialColor)
{
}

int StoneSlabBlock::Variant::getData() { return mData; }
int StoneSlabBlock::Variant::getTranslationKey() { return mTranslationKey; }

StoneSlabBlock::Variant* StoneSlabBlock::Variant::byData(int data)
{
    if (data < 0 || data > 7)
    {
        data = 0;
    }
    return gStoneSlabVariants[data];
}

const MaterialColor* StoneSlabBlock::Variant::getColor() { return mColor; }

std::wstring StoneSlabBlock::Variant::toString() const { return mName; }
std::wstring StoneSlabBlock::Variant::getSerializedName() const { return mName; }

void StoneSlabBlock::blockStaticCtor()
{
    gStoneSlabVariants[0] = new Variant(0, MaterialColor::STONE, L"stone", 762025956);
    gStoneSlabVariants[1] = new Variant(1, MaterialColor::SAND, L"sandstone", -293529481);
    gStoneSlabVariants[2] = new Variant(2, MaterialColor::WOOD, L"wood_old", -1214914228);
    gStoneSlabVariants[3] = new Variant(3, MaterialColor::STONE, L"cobblestone", -1579406718);
    gStoneSlabVariants[4] = new Variant(4, MaterialColor::COLOR_RED, L"brick", -1553598690);
    gStoneSlabVariants[5] = new Variant(5, MaterialColor::STONE, L"stone_brick", -227356180);
    gStoneSlabVariants[6] = new Variant(6, MaterialColor::NETHER, L"nether_brick", -1524297450);
    gStoneSlabVariants[7] = new Variant(7, MaterialColor::QUARTZ, L"quartz", 684318662);

    boost::unordered_set<Variant*> values;
    for (int i = 0; i < 8; ++i)
    {
        values.insert(gStoneSlabVariants[i]);
    }
    gStoneSlabVariantProperty = EnumProperty<Variant*>::create(
        L"variant", typeid(Variant*), values, Predicates<Variant*>::alwaysTrue);
}

StoneSlabBlock::StoneSlabBlock()
    : HalfSlabBlock(Material::STONE)
{
}

StoneSlabBlock::~StoneSlabBlock() {}

void StoneSlabBlock::DerivedInit()
{
    HalfSlabBlock::DerivedInit();
    const BlockState* state = mStateDefinition->any();
    if (isFullSize())
    {
        TypedBoxed<bool> seamless(false);
        state = state->setBoxedValue(gStoneSlabSeamlessProperty, &seamless);
    }
    else
    {
        TypedBoxed<Half*> half(BOTTOM);
        state = state->setBoxedValue(TYPE, &half);
    }
    TypedBoxed<Variant*> variant(gStoneSlabVariants[0]);
    registerDefaultState(state->setBoxedValue(gStoneSlabVariantProperty, &variant));
}

Item* StoneSlabBlock::getResource(const BlockState* state, Random*, int)
{
    return Item::byBlock(Blocks::stone_slab);
}

not_null_ptr<ItemInstance> StoneSlabBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    int data = state->getValue(gStoneSlabVariantProperty)->getData();
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stone_slab, 1, data));
}

int StoneSlabBlock::getAuxName(int auxValue)
{
    return Variant::byData(auxValue)->getTranslationKey();
}

Property* StoneSlabBlock::getTypeProperty()
{
    return gStoneSlabVariantProperty;
}

Boxed* StoneSlabBlock::getTypeForItem(not_null_ptr<ItemInstance> itemInstance)
{
    return new TypedBoxed<Variant*>(Variant::byData(itemInstance->getAuxValue() & 7));
}

const BlockState* StoneSlabBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> variant(Variant::byData(data & 7));
    const BlockState* state = defaultBlockState()->setBoxedValue(gStoneSlabVariantProperty, &variant);
    if (!isFullSize())
    {
        Half* halfValue = (data & 8) == 0 ? BOTTOM : TOP;
        TypedBoxed<Half*> half(halfValue);
        state = state->setBoxedValue(TYPE, &half);
    }
    return state;
}

int StoneSlabBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(gStoneSlabVariantProperty)->getData();
    if (!isFullSize() && state->getValue(HalfSlabBlock::TYPE) == HalfSlabBlock::TOP)
    {
        data |= 8;
    }
    return data;
}

BlockStateDefinition* StoneSlabBlock::createBlockStateDefinition()
{
    const Property* properties[2] = {
        isFullSize() ? gStoneSlabSeamlessProperty : HalfSlabBlock::TYPE,
        gStoneSlabVariantProperty
    };
    return new BlockStateDefinition(this, properties, 2);
}

int StoneSlabBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(gStoneSlabVariantProperty)->getData();
}

const MaterialColor* StoneSlabBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(gStoneSlabVariantProperty)->getColor();
}

TextureAtlasSprite* StoneSlabBlock::getTexture(const Direction* direction, const BlockState* state)
{
    StoneSlabBlock::Variant* variant = state->getValue(gStoneSlabVariantProperty);
    if (variant->getData() == 0)
    {
        return direction == Direction::UP || direction == Direction::DOWN ? mIcon : mSideTexture;
    }
    switch (variant->getData())
    {
    case 1:
        return Blocks::sandstone->getTexture(direction);
    case 2:
        return Blocks::planks->getTexture(direction);
    case 3:
        return Blocks::cobblestone->getTexture(direction);
    case 4:
        return Blocks::brick_block->getTexture(direction);
    case 5:
        return Blocks::stonebrick->getTexture(direction);
    case 6:
        return Blocks::netherrack->getTexture(Direction::UP);
    case 7:
        return Blocks::quartz_block->getTexture(direction);
    }
    return mIcon;
}

void StoneSlabBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"stoneslab_top");
    mSideTexture = iconRegister->registerIcon(L"stoneslab_side");
}
