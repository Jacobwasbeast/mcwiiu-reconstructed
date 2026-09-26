#include "game/world/block/StoneBrickBlock.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Material.hpp"

static StoneBrickBlock::Variant gStoneBrickVariant0(0, L"stonebrick", 0);
static StoneBrickBlock::Variant gStoneBrickVariant1(1, L"mossy_stonebrick", 1);
static StoneBrickBlock::Variant gStoneBrickVariant2(2, L"cracked_stonebrick", 2);
static StoneBrickBlock::Variant gStoneBrickVariant3(3, L"chiseled_stonebrick", 3);
static StoneBrickBlock::Variant* gStoneBrickVariants[4] = {
    &gStoneBrickVariant0, &gStoneBrickVariant1, &gStoneBrickVariant2, &gStoneBrickVariant3
};
StoneBrickBlock::Variant* StoneBrickBlock::Variant::DEFAULT = &gStoneBrickVariant0;
StoneBrickBlock::Variant* StoneBrickBlock::Variant::MOSSY = &gStoneBrickVariant1;
StoneBrickBlock::Variant* StoneBrickBlock::Variant::CRACKED = &gStoneBrickVariant2;
StoneBrickBlock::Variant* StoneBrickBlock::Variant::CHISELED = &gStoneBrickVariant3;

int StoneBrickBlock::DEFAULT_META = 0;
int StoneBrickBlock::MOSSY_META = 0;
int StoneBrickBlock::CRACKED_META = 0;
int StoneBrickBlock::CHISELED_META = 0;
EnumProperty<StoneBrickBlock::Variant*>* StoneBrickBlock::VARIANT = NULL;

StoneBrickBlock::Variant::Variant(int data, const std::wstring& name, int translationKey)
    : mData(data), mName(name), mTranslationKey(translationKey)
{
}

int StoneBrickBlock::Variant::getData()
{
    return mData;
}

StoneBrickBlock::Variant* StoneBrickBlock::Variant::byData(int data)
{
    if (data < 0 || data > 3)
    {
        data = 0;
    }
    return gStoneBrickVariants[data];
}

int StoneBrickBlock::Variant::getTranslationKey()
{
    return mTranslationKey;
}

std::wstring StoneBrickBlock::Variant::toString() const
{
    return mName;
}

std::wstring StoneBrickBlock::Variant::getSerializedName() const
{
    return mName;
}

void StoneBrickBlock::blockStaticCtor()
{
    if (VARIANT == NULL)
    {
        boost::unordered_set<Variant*> values;
        values.insert(gStoneBrickVariants[0]);
        values.insert(gStoneBrickVariants[1]);
        values.insert(gStoneBrickVariants[2]);
        values.insert(gStoneBrickVariants[3]);
    VARIANT = EnumProperty<Variant*>::create(
            L"variant", typeid(Variant*), values, Predicates<Variant*>::alwaysTrue);
    }
    DEFAULT_META = gStoneBrickVariants[0]->getData();
    MOSSY_META = gStoneBrickVariants[1]->getData();
    CRACKED_META = gStoneBrickVariants[2]->getData();
    CHISELED_META = gStoneBrickVariants[3]->getData();
}

StoneBrickBlock::StoneBrickBlock()
    : Block(Material::STONE)
{
    DerivedInit();
    TypedBoxed<Variant*> value(Variant::DEFAULT);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(VARIANT, &value));
    for (int i = 0; i < 4; ++i)
    {
        mTextures[i] = NULL;
    }
}

StoneBrickBlock::~StoneBrickBlock()
{
}

int StoneBrickBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

const BlockState* StoneBrickBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> value(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(VARIANT, &value);
}

int StoneBrickBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

BlockStateDefinition* StoneBrickBlock::createBlockStateDefinition()
{
    const Property* properties[] = { VARIANT };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* StoneBrickBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return mTextures[state->getValue(VARIANT)->getData()];
}

void StoneBrickBlock::registerIcons(IconRegister* iconRegister)
{
    std::wstring base = getIconName();
    mTextures[0] = iconRegister->registerIcon(base);
    mTextures[1] = iconRegister->registerIcon(base + L"_mossy");
    mTextures[2] = iconRegister->registerIcon(base + L"_cracked");
    mTextures[3] = iconRegister->registerIcon(base + L"_carved");
}

int StoneBrickBlock::getDescriptionId(int auxValue)
{
    return Variant::byData(auxValue)->getTranslationKey();
}
