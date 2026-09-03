#include "game/world/block/StoneBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/item/Item.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

static StoneBlock::Variant* gStoneVariantPointers[7] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };
static EnumProperty<StoneBlock::Variant*>* gStoneVariantProperty = NULL;

StoneBlock::Variant::Variant(int data, const MaterialColor* materialColor, const std::wstring& name, int nameId, int descriptionId, const std::wstring& serializedName, bool natural)
    : mData(data), mName(name), mNameId(nameId), mDescriptionId(descriptionId), mMaterialColor(materialColor), mSerializedName(serializedName), mNatural(natural), mTexture(NULL)
{
}

const MaterialColor* StoneBlock::Variant::getMaterialColor() { return mMaterialColor; }
int StoneBlock::Variant::getData() { return mData; }
StoneBlock::Variant* StoneBlock::Variant::byData(int data) { return data > 6 ? gStoneVariantPointers[0] : gStoneVariantPointers[data]; }
TextureAtlasSprite* StoneBlock::Variant::getTexture() { return mTexture; }
void StoneBlock::Variant::registerIcons(IconRegister* iconRegister) { mTexture = iconRegister->registerIcon(mSerializedName); }
std::wstring StoneBlock::Variant::toString() const { return mName; }
std::wstring StoneBlock::Variant::getSerializedName() const { return mName; }
int StoneBlock::Variant::getNameId() { return mNameId; }
int StoneBlock::Variant::getDescriptionId() { return mDescriptionId; }
bool StoneBlock::Variant::isNatural() const { return mNatural; }
std::wstring StoneBlock::Variant::getName() { return mName; }

void StoneBlock::blockStaticCtor()
{
    gStoneVariantPointers[0] = new Variant(0, MaterialColor::STONE, L"stone", 0x8bad4dac, 0x46782239, L"stone", true);
    gStoneVariantPointers[1] = new Variant(1, MaterialColor::STONE, L"granite", 0x19244f91, 0x91114554, L"stone_granite", true);
    gStoneVariantPointers[2] = new Variant(2, MaterialColor::STONE, L"smooth_granite", 0x567cedee, 0x25e3fdc3, L"stone_granite_smooth", false);
    gStoneVariantPointers[3] = new Variant(3, MaterialColor::STONE, L"diorite", 0x77f221b3, 0x05e93db6, L"stone_diorite", true);
    gStoneVariantPointers[4] = new Variant(4, MaterialColor::STONE, L"smooth_diorite", 0xf1c0b35c, 0x2a4cccf1, L"stone_diorite_smooth", false);
    gStoneVariantPointers[5] = new Variant(5, MaterialColor::STONE, L"andesite", 0xbd013dc6, 0xf2a753fb, L"stone_andesite", true);
    gStoneVariantPointers[6] = new Variant(6, MaterialColor::STONE, L"smooth_andesite", 0x1bb05261, 0x47be1614, L"stone_andesite_smooth", false);
    boost::unordered_set<Variant*> values;
    for (int i = 0; i < 7; ++i) values.insert(gStoneVariantPointers[i]);
    gStoneVariantProperty = EnumProperty<Variant*>::create(L"variant", typeid(Variant*), values, Predicates<Variant*>::alwaysTrue);
}

StoneBlock::StoneBlock() : Block(Material::STONE)
{
    DerivedInit();
    TypedBoxed<Variant*> value(gStoneVariantPointers[0]);
    registerDefaultState(defaultBlockState()->setBoxedValue(gStoneVariantProperty, &value));
}

StoneBlock::~StoneBlock() {}
std::wstring StoneBlock::getName() { return L10N::GetString(0x8bad4dac); }

const MaterialColor* StoneBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(gStoneVariantProperty)->getMaterialColor();
}

Item* StoneBlock::getResource(const BlockState* state, Random*, int)
{
    return Item::byBlock(state->getValue(gStoneVariantProperty) == gStoneVariantPointers[0] ? Blocks::stone : Blocks::cobblestone);
}

int StoneBlock::getSpawnResourcesAuxValue(const BlockState* state) { return state->getValue(gStoneVariantProperty)->getData(); }
const BlockState* StoneBlock::getBlockState(int data) { TypedBoxed<Variant*> value(Variant::byData(data)); return defaultBlockState()->setBoxedValue(gStoneVariantProperty, &value); }
int StoneBlock::convertBlockStateToLegacyData(const BlockState* state) { return state->getValue(gStoneVariantProperty)->getData(); }
BlockStateDefinition* StoneBlock::createBlockStateDefinition() { const Property* properties[1] = { gStoneVariantProperty }; return new BlockStateDefinition(this, properties, 1); }
TextureAtlasSprite* StoneBlock::getTexture(const Direction*, const BlockState* state) { return state->getValue(gStoneVariantProperty)->getTexture(); }
void StoneBlock::registerIcons(IconRegister* iconRegister) { for (int i = 0; i < 7; ++i) gStoneVariantPointers[i]->registerIcons(iconRegister); }
