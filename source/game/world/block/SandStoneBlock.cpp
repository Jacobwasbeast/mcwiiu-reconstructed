#include "game/world/block/SandStoneBlock.hpp"

#include <cstring>
#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

static SandStoneBlock::Variant gSandStoneVariant0(0, L"sandstone", 0);
static SandStoneBlock::Variant gSandStoneVariant1(1, L"chiseled_sandstone", 1);
static SandStoneBlock::Variant gSandStoneVariant2(2, L"smooth_sandstone", 2);
static SandStoneBlock::Variant* gSandStoneVariants[3] = {
    &gSandStoneVariant0, &gSandStoneVariant1, &gSandStoneVariant2
};
AbstractProperty<SandStoneBlock::Variant*>* SandStoneBlock::TYPE = NULL;
SandStoneBlock::Variant* SandStoneBlock::SANDSTONE = &gSandStoneVariant0;
SandStoneBlock::Variant* SandStoneBlock::CHISELED = &gSandStoneVariant1;
SandStoneBlock::Variant* SandStoneBlock::SMOOTH = &gSandStoneVariant2;
static const MaterialColor* gSandStoneMapColor = NULL;

static Predicate<SandStoneBlock::Variant*>* gSandStoneTypePredicate =
    new Predicates<SandStoneBlock::Variant*>::ConstantPredicate(true);
static std::wstring gSandStoneTopTexture = L"sandstone_top";
static std::wstring gSandStoneBottomTexture = L"sandstone_bottom";
static std::wstring gSandStoneSideTexture = L"sandstone_side";
static std::wstring gSandStoneCarvedTexture = L"sandstone_carved";
static std::wstring gSandStoneSmoothTexture = L"sandstone_smooth";

SandStoneBlock::Variant::Variant(int data, const std::wstring& name, int translationKey)
    : mData(data), mName(name), mTranslationKey(translationKey)
{
}

int SandStoneBlock::Variant::getData()
{
    return mData;
}

SandStoneBlock::Variant* SandStoneBlock::Variant::byData(int data)
{
    if (data < 0 || data > 2)
    {
        data = 0;
    }
    return gSandStoneVariants[data];
}

std::wstring SandStoneBlock::Variant::toString() const
{
    return mName;
}

std::wstring SandStoneBlock::Variant::getSerializedName() const
{
    return mName;
}

int SandStoneBlock::Variant::getTranslationKey()
{
    return mTranslationKey;
}

void SandStoneBlock::blockStaticCtor()
{
    std::wstring name = L"type";
    EnumProperty<Variant*>* type;
    {
        boost::unordered_set<Variant*> values;
        values.insert(gSandStoneVariants[0]);
        values.insert(gSandStoneVariants[1]);
        values.insert(gSandStoneVariants[2]);
        type = EnumProperty<Variant*>::create(
            name, typeid(Variant), values, gSandStoneTypePredicate);
    }
    TYPE = type;
}

SandStoneBlock::SandStoneBlock()
    : Block(Material::STONE)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(TYPE, SANDSTONE));
    memset(mTextures, 0, sizeof(mTextures));
}

int SandStoneBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(TYPE)->getData();
}

const MaterialColor* SandStoneBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return gSandStoneMapColor;
}

const BlockState* SandStoneBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(TYPE, Variant::byData(data));
}

int SandStoneBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(TYPE)->getData();
}

BlockStateDefinition* SandStoneBlock::createBlockStateDefinition()
{
    static const Property* properties[1] = { TYPE };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* SandStoneBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Variant* variant = state->getValue(TYPE);
    if (direction != Direction::UP && direction == Direction::DOWN &&
        variant != gSandStoneVariants[1] && variant != gSandStoneVariants[2])
    {
        return mBottomTexture;
    }
    if (direction == Direction::UP || direction == Direction::DOWN)
    {
        return mTopTexture;
    }
    return mTextures[variant->getData()];
}

void SandStoneBlock::registerIcons(IconRegister* iconRegister)
{
    mTextures[0] = iconRegister->registerIcon(gSandStoneSideTexture);
    mTextures[1] = iconRegister->registerIcon(gSandStoneCarvedTexture);
    mTextures[2] = iconRegister->registerIcon(gSandStoneSmoothTexture);
    mTopTexture = iconRegister->registerIcon(gSandStoneTopTexture);
    mBottomTexture = iconRegister->registerIcon(gSandStoneBottomTexture);
}
