#include "game/world/block/NewStoneSlabBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
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
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

NewStoneSlabBlock::Variant* NewStoneSlabBlock::Variant::RED_SANDSTONE = NULL;
EnumProperty<NewStoneSlabBlock::Variant*>* NewStoneSlabBlock::VARIANT = NULL;
static Property* gNewStoneSlabSeamlessProperty = BooleanProperty::create(L"seamless");

NewStoneSlabBlock::Variant::Variant(int data, const std::wstring& name, int translationKey, const MaterialColor* materialColor)
    : mData(data),
      mName(name),
      mTranslationKey(translationKey),
      mMaterialColor(materialColor)
{
}

NewStoneSlabBlock::Variant* NewStoneSlabBlock::Variant::byData(int data)
{
    return Variant::RED_SANDSTONE;
}

int NewStoneSlabBlock::Variant::getData()
{
    return mData;
}

const MaterialColor* NewStoneSlabBlock::Variant::getMaterialColor()
{
    return mMaterialColor;
}

std::wstring NewStoneSlabBlock::Variant::toString() const
{
    return mName;
}

std::wstring NewStoneSlabBlock::Variant::getSerializedName() const
{
    return mName;
}

NewStoneSlabBlock::NewStoneSlabBlock()
    : HalfSlabBlock(Material::STONE)
{
}

NewStoneSlabBlock::~NewStoneSlabBlock()
{
}

void NewStoneSlabBlock::DerivedInit()
{
    HalfSlabBlock::DerivedInit();

    const BlockState* state = mStateDefinition->any();
    if (isFullSize())
    {
        TypedBoxed<bool> seamless(false);
        state = state->setBoxedValue(gNewStoneSlabSeamlessProperty, &seamless);
    }
    else
    {
        TypedBoxed<Half*> half(BOTTOM);
        state = state->setBoxedValue(TYPE, &half);
    }

    TypedBoxed<Variant*> variant(Variant::RED_SANDSTONE);
    registerDefaultState(state->setBoxedValue(VARIANT, &variant));
}

void NewStoneSlabBlock::blockStaticCtor()
{
    Variant::RED_SANDSTONE = new Variant(
        0, L"red_sandstone", 0x4E7D3907, SandBlock::Variant::byData(1)->getMaterialColor());

    boost::unordered_set<Variant*> values;
    values.insert(Variant::RED_SANDSTONE);
    VARIANT = EnumProperty<Variant*>::create(
        L"variant", typeid(Variant*), values, Predicates<Variant*>::alwaysTrue);
}

std::wstring NewStoneSlabBlock::getName()
{
    return L10N::GetString(IDS_TILE_STONESLAB);
}

Item* NewStoneSlabBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Item::byBlock(Blocks::stone_slab2);
}

not_null_ptr<ItemInstance> NewStoneSlabBlock::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stone_slab2, 1, data));
}

int NewStoneSlabBlock::getAuxName(int auxValue)
{
    return getDescriptionId(auxValue);
}

Property* NewStoneSlabBlock::getTypeProperty()
{
    return VARIANT;
}

Boxed* NewStoneSlabBlock::getTypeForItem(not_null_ptr<ItemInstance> itemInstance)
{
    return new TypedBoxed<Variant*>(Variant::byData(itemInstance->getAuxValue() & 7));
}

const BlockState* NewStoneSlabBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> variant(Variant::byData(data & 7));
    const BlockState* state = defaultBlockState()->setBoxedValue(VARIANT, &variant);

    if (!isFullSize())
    {
        Half* halfValue = (data & 8) == 0 ? BOTTOM : TOP;
        TypedBoxed<Half*> half(halfValue);
        state = state->setBoxedValue(TYPE, &half);
    }
    else
    {
        TypedBoxed<bool> seamless((data & 8) != 0);
        state = state->setBoxedValue(gNewStoneSlabSeamlessProperty, &seamless);
    }

    return state;
}

int NewStoneSlabBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();

    if (!isFullSize() && state->getValue(TYPE) == TOP)
    {
        data |= 8;
    }
    else if (isFullSize() && state->getValue(static_cast<const AbstractProperty<bool>*>(gNewStoneSlabSeamlessProperty)))
    {
        data |= 8;
    }

    return data;
}

BlockStateDefinition* NewStoneSlabBlock::createBlockStateDefinition()
{
    const Property* properties[2] = {
        isFullSize() ? gNewStoneSlabSeamlessProperty : TYPE,
        VARIANT
    };
    return new BlockStateDefinition(this, properties, 2);
}

const MaterialColor* NewStoneSlabBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return state->getValue(VARIANT)->getMaterialColor();
}

int NewStoneSlabBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

TextureAtlasSprite* NewStoneSlabBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction != Direction::UP && direction != Direction::DOWN)
    {
        return mSideTexture;
    }

    return mIcon;
}

void NewStoneSlabBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"red_sandstone_top");
    mSideTexture = iconRegister->registerIcon(L"red_sandstone_normal");
}
