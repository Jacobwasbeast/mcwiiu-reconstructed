#include "game/world/block/PrismarineBlock.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

PrismarineBlock::Variant* PrismarineBlock::Variant::ROUGH =
    new Variant(0, L"prismarine", 0xc36ed73b, 0x4d2dbbd6, L"prismarine_rough");
PrismarineBlock::Variant* PrismarineBlock::Variant::BRICKS =
    new Variant(1, L"prismarine_bricks", 0xf0338e76, 0xc6915853, L"prismarine_bricks");
PrismarineBlock::Variant* PrismarineBlock::Variant::DARK =
    new Variant(2, L"dark_prismarine", 0x704b6b38, 0xc9e0c23d, L"prismarine_dark");

static PrismarineBlock::Variant* gPrismarineVariants[3] = {
    PrismarineBlock::Variant::ROUGH,
    PrismarineBlock::Variant::BRICKS,
    PrismarineBlock::Variant::DARK
};
EnumProperty<PrismarineBlock::Variant*>* PrismarineBlock::VARIANT = NULL;
static Predicate<PrismarineBlock::Variant*>* gPrismarineVariantPredicate =
    new Predicates<PrismarineBlock::Variant*>::ConstantPredicate(true);
static int gPrismarineVariantData[3] = { -1, -1, -1 };

PrismarineBlock::Variant::Variant(int data, const std::wstring& name, int nameId,
                                  int descriptionId, const std::wstring& serializedName)
    : mData(data),
      mName(name),
      mNameId(nameId),
      mDescriptionId(descriptionId),
      mSerializedName(serializedName)
{
}

std::wstring PrismarineBlock::Variant::toString() const
{
    return mName;
}

std::wstring PrismarineBlock::Variant::getSerializedName() const
{
    return mSerializedName;
}

int PrismarineBlock::Variant::getData()
{
    return mData;
}

PrismarineBlock::Variant* PrismarineBlock::Variant::byData(int data)
{
    if (data > 2 || data < 0)
    {
        data = 0;
    }

    return gPrismarineVariants[data];
}

TextureAtlasSprite* PrismarineBlock::Variant::getTexture()
{
    return mTexture;
}

void PrismarineBlock::Variant::registerIcons(IconRegister* iconRegister)
{
    mTexture = iconRegister->registerIcon(mSerializedName);
}

int PrismarineBlock::Variant::getNameId()
{
    return mNameId;
}

int PrismarineBlock::Variant::getDescriptionId()
{
    return mDescriptionId;
}

void PrismarineBlock::blockStaticCtor()
{
    std::wstring name = L"variant";
    boost::unordered_set<Variant*> values;
    values.insert(gPrismarineVariants[0]);
    values.insert(gPrismarineVariants[1]);
    values.insert(gPrismarineVariants[2]);
    VARIANT = EnumProperty<Variant*>::create(name, typeid(Variant), values, gPrismarineVariantPredicate);
    gPrismarineVariantData[0] = gPrismarineVariants[0]->getData();
    gPrismarineVariantData[1] = gPrismarineVariants[1]->getData();
    gPrismarineVariantData[2] = gPrismarineVariants[2]->getData();
}

PrismarineBlock::PrismarineBlock()
    : Block(Material::STONE)
{
    DerivedInit();
    TypedBoxed<Variant*> value(Variant::ROUGH);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(VARIANT, &value));
}

PrismarineBlock::~PrismarineBlock()
{
}

std::wstring PrismarineBlock::getName()
{
    return L10N::GetString(0xc36ed73b);
}

const MaterialColor* PrismarineBlock::getMapColor(const BlockState* state, LevelSource*, const BlockPos&)
{
    return state->getValue(VARIANT) == Variant::ROUGH
        ? MaterialColor::COLOR_CYAN
        : MaterialColor::DIAMOND;
}

int PrismarineBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

int PrismarineBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

BlockStateDefinition* PrismarineBlock::createBlockStateDefinition()
{
    const Property* properties[] = { VARIANT };
    return new BlockStateDefinition(this, properties, 1);
}

const BlockState* PrismarineBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> value(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(VARIANT, &value);
}

TextureAtlasSprite* PrismarineBlock::getTexture(const Direction*, const BlockState* state)
{
    return state->getValue(VARIANT)->getTexture();
}

void PrismarineBlock::registerIcons(IconRegister* iconRegister)
{
    gPrismarineVariants[0]->registerIcons(iconRegister);
    gPrismarineVariants[1]->registerIcons(iconRegister);
    gPrismarineVariants[2]->registerIcons(iconRegister);
}
