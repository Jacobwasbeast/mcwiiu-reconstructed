#include "game/world/block/StoneSlabBlock.hpp"
#include "game/core/StringIds.hpp"

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

StoneSlabBlock::Variant* StoneSlabBlock::Variant::STONE = NULL;
StoneSlabBlock::Variant* StoneSlabBlock::Variant::SAND = NULL;
StoneSlabBlock::Variant* StoneSlabBlock::Variant::WOOD = NULL;
StoneSlabBlock::Variant* StoneSlabBlock::Variant::COBBLESTONE = NULL;
StoneSlabBlock::Variant* StoneSlabBlock::Variant::BRICK = NULL;
StoneSlabBlock::Variant* StoneSlabBlock::Variant::SMOOTHBRICK = NULL;
StoneSlabBlock::Variant* StoneSlabBlock::Variant::NETHERBRICK = NULL;
StoneSlabBlock::Variant* StoneSlabBlock::Variant::QUARTZ = NULL;

static StoneSlabBlock::Variant* gStoneSlabVariants[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
EnumProperty<StoneSlabBlock::Variant*>* StoneSlabBlock::VARIANT = NULL;
static AbstractProperty<bool>* gStoneSlabSeamlessProperty =
    static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"seamless"));

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
    Variant::STONE = new Variant(0, MaterialColor::STONE, L"stone", IDS_TILE_STONESLAB_STONE);
    Variant::SAND = new Variant(1, MaterialColor::SAND, L"sandstone", 0xEE811877);
    Variant::WOOD = new Variant(2, MaterialColor::WOOD, L"wood_old", 0xB795E14C);
    Variant::COBBLESTONE = new Variant(3, MaterialColor::STONE, L"cobblestone", 0xA1DC2A82);
    Variant::BRICK = new Variant(4, MaterialColor::COLOR_RED, L"brick", 0xA365F71E);
    Variant::SMOOTHBRICK = new Variant(5, MaterialColor::STONE, L"stone_brick", 0xF272D1EC);
    Variant::NETHERBRICK = new Variant(6, MaterialColor::NETHER, L"nether_brick", 0xA5251116);
    Variant::QUARTZ = new Variant(7, MaterialColor::QUARTZ, L"quartz", 0x28C9DFC6);
    gStoneSlabVariants[0] = Variant::STONE;
    gStoneSlabVariants[1] = Variant::SAND;
    gStoneSlabVariants[2] = Variant::WOOD;
    gStoneSlabVariants[3] = Variant::COBBLESTONE;
    gStoneSlabVariants[4] = Variant::BRICK;
    gStoneSlabVariants[5] = Variant::SMOOTHBRICK;
    gStoneSlabVariants[6] = Variant::NETHERBRICK;
    gStoneSlabVariants[7] = Variant::QUARTZ;

    boost::unordered_set<Variant*> values;
    for (int i = 0; i < 8; ++i)
    {
        values.insert(gStoneSlabVariants[i]);
    }
    VARIANT = EnumProperty<Variant*>::create(
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
    registerDefaultState(state->setBoxedValue(VARIANT, &variant));
}

Item* StoneSlabBlock::getResource(const BlockState* state, Random*, int)
{
    return Item::byBlock(Blocks::stone_slab);
}

not_null_ptr<ItemInstance> StoneSlabBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stone_slab, 1, data));
}

int StoneSlabBlock::getAuxName(int auxValue)
{
    return Variant::byData(auxValue)->getTranslationKey();
}

Property* StoneSlabBlock::getTypeProperty()
{
    return VARIANT;
}

Boxed* StoneSlabBlock::getTypeForItem(not_null_ptr<ItemInstance> itemInstance)
{
    return new TypedBoxed<Variant*>(Variant::byData(itemInstance->getAuxValue() & 7));
}

const BlockState* StoneSlabBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> variant(Variant::byData(data & 7));
    const BlockState* state = defaultBlockState()->setBoxedValue(VARIANT, &variant);
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
    int data = state->getValue(VARIANT)->getData();
    if (isFullSize())
    {
        if (state->getValue(gStoneSlabSeamlessProperty))
        {
            data |= 8;
        }
    }
    else if (state->getValue(HalfSlabBlock::TYPE) == HalfSlabBlock::TOP)
    {
        data |= 8;
    }
    return data;
}

BlockStateDefinition* StoneSlabBlock::createBlockStateDefinition()
{
    const Property* properties[2] = {
        isFullSize() ? static_cast<const Property*>(gStoneSlabSeamlessProperty)
                     : static_cast<const Property*>(HalfSlabBlock::TYPE),
        VARIANT
    };
    return new BlockStateDefinition(this, properties, 2);
}

int StoneSlabBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

const MaterialColor* StoneSlabBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(VARIANT)->getColor();
}

TextureAtlasSprite* StoneSlabBlock::getTexture(const Direction* direction, const BlockState* state)
{
    StoneSlabBlock::Variant* variant = state->getValue(VARIANT);
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
        return Blocks::nether_brick->getTexture(Direction::UP);
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
