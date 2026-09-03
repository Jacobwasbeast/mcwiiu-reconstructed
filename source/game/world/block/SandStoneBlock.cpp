#include "game/world/block/SandStoneBlock.hpp"

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
static EnumProperty<SandStoneBlock::Variant*>* gSandStoneVariantProperty = NULL;
static const MaterialColor* gSandStoneMapColor = NULL;

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
    if (data > 2)
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

std::wstring SandStoneBlock::Variant::getName()
{
    return mName;
}

void SandStoneBlock::blockStaticCtor()
{
    boost::unordered_set<Variant*> values;
    values.insert(gSandStoneVariants[0]);
    values.insert(gSandStoneVariants[1]);
    values.insert(gSandStoneVariants[2]);
    gSandStoneVariantProperty = EnumProperty<Variant*>::create(
        L"type", typeid(Variant*), values, Predicates<Variant*>::alwaysTrue);
}

SandStoneBlock::SandStoneBlock()
    : Block(Material::SAND)
{
    DerivedInit();
    TypedBoxed<Variant*> value(gSandStoneVariants[0]);
    registerDefaultState(defaultBlockState()->setBoxedValue(gSandStoneVariantProperty, &value));
    for (int i = 0; i < 5; ++i)
    {
        mTextures[i] = NULL;
    }
}

SandStoneBlock::~SandStoneBlock()
{
}

int SandStoneBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(gSandStoneVariantProperty)->getData();
}

const MaterialColor* SandStoneBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return gSandStoneMapColor;
}

const BlockState* SandStoneBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> value(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(gSandStoneVariantProperty, &value);
}

int SandStoneBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gSandStoneVariantProperty)->getData();
}

BlockStateDefinition* SandStoneBlock::createBlockStateDefinition()
{
    const Property* properties[1];
    properties[0] = gSandStoneVariantProperty;
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* SandStoneBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Variant* variant = state->getValue(gSandStoneVariantProperty);
    if (direction != Direction::UP && direction == Direction::DOWN &&
        variant != gSandStoneVariants[1] && variant != gSandStoneVariants[2])
    {
        return mTextures[4];
    }
    if (direction == Direction::UP || direction == Direction::DOWN)
    {
        return mTextures[3];
    }
    return mTextures[variant->getData()];
}

void SandStoneBlock::registerIcons(IconRegister* iconRegister)
{
    mTextures[0] = iconRegister->registerIcon(L"sandstone");
    mTextures[1] = iconRegister->registerIcon(L"sandstone_carved");
    mTextures[2] = iconRegister->registerIcon(L"sandstone_smooth");
    mTextures[3] = iconRegister->registerIcon(L"sandstone_top");
    mTextures[4] = iconRegister->registerIcon(L"sandstone_bottom");
}
