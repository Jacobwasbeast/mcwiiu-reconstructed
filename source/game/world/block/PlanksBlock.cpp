#include "game/world/block/PlanksBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/IconRegister.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

PlanksBlock::Variant* PlanksBlock::Variant::OAK =
    new PlanksBlock::Variant(Variant::eDataOak, L"oak", Variant::eTranslationKeyOak, MaterialColor::WOOD);
PlanksBlock::Variant* PlanksBlock::Variant::SPRUCE =
    new PlanksBlock::Variant(Variant::eDataSpruce, L"spruce", Variant::eTranslationKeySpruce, MaterialColor::PODZOL);
PlanksBlock::Variant* PlanksBlock::Variant::BIRCH =
    new PlanksBlock::Variant(Variant::eDataBirch, L"birch", Variant::eTranslationKeyBirch, MaterialColor::SAND);
PlanksBlock::Variant* PlanksBlock::Variant::JUNGLE =
    new PlanksBlock::Variant(Variant::eDataJungle, L"jungle", Variant::eTranslationKeyJungle, MaterialColor::DIRT);
PlanksBlock::Variant* PlanksBlock::Variant::ACACIA =
    new PlanksBlock::Variant(Variant::eDataAcacia, L"acacia", Variant::eTranslationKeyAcacia, MaterialColor::COLOR_ORANGE);
PlanksBlock::Variant* PlanksBlock::Variant::DARK_OAK =
    new PlanksBlock::Variant(Variant::eDataDarkOak, L"dark_oak", Variant::eTranslationKeyDarkOak, MaterialColor::COLOR_BROWN);

PlanksBlock::Variant* gPlankVariants[6] = {
    PlanksBlock::Variant::OAK,
    PlanksBlock::Variant::SPRUCE,
    PlanksBlock::Variant::BIRCH,
    PlanksBlock::Variant::JUNGLE,
    PlanksBlock::Variant::ACACIA,
    PlanksBlock::Variant::DARK_OAK
};
EnumProperty<PlanksBlock::Variant*>* PlanksBlock::VARIANT = NULL;

class PlanksVariantPredicate : public Predicate<PlanksBlock::Variant*>
{
public:
    virtual bool apply(PlanksBlock::Variant* value) const
    {
        return value != NULL;
    }
};

PlanksBlock::Variant::Variant(int data, const std::wstring& name, int translationKey, const MaterialColor* materialColor)
    : mData(data), mName(name), mTranslationKey(translationKey), mMaterialColor(materialColor)
{
}

std::wstring PlanksBlock::Variant::toString() const
{
    return mName;
}

std::wstring PlanksBlock::Variant::getSerializedName() const
{
    return mName;
}

int PlanksBlock::Variant::getData()
{
    return mData;
}

int PlanksBlock::Variant::getTranslationKey()
{
    return mTranslationKey;
}

const MaterialColor* PlanksBlock::Variant::getMaterialColor()
{
    return mMaterialColor;
}

PlanksBlock::Variant* PlanksBlock::Variant::byData(int data)
{
    switch (data)
    {
    case Variant::eDataSpruce: return Variant::SPRUCE;
    case Variant::eDataBirch: return Variant::BIRCH;
    case Variant::eDataJungle: return Variant::JUNGLE;
    case Variant::eDataAcacia: return Variant::ACACIA;
    case Variant::eDataDarkOak: return Variant::DARK_OAK;
    case Variant::eDataOak:
    default: return Variant::OAK;
    }
}

void PlanksBlock::blockStaticCtor()
{
    boost::unordered_set<Variant*> values;
    values.insert(Variant::OAK);
    values.insert(Variant::SPRUCE);
    values.insert(Variant::BIRCH);
    values.insert(Variant::JUNGLE);
    values.insert(Variant::ACACIA);
    values.insert(Variant::DARK_OAK);
    PlanksVariantPredicate predicate;
    VARIANT = EnumProperty<Variant*>::create(L"variant", typeid(Variant), values, &predicate);
}

PlanksBlock::PlanksBlock()
    : Block(Material::WOOD)
{
    DerivedInit();
    TypedBoxed<Variant*> value(Variant::OAK);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(VARIANT, &value));
    for (int i = 0; i < 6; ++i)
    {
        mIcons[i] = NULL;
    }
}

int PlanksBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

const BlockState* PlanksBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> value(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(VARIANT, &value);
}

const MaterialColor* PlanksBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return state->getValue(VARIANT)->getMaterialColor();
}

int PlanksBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

BlockStateDefinition* PlanksBlock::createBlockStateDefinition()
{
    const Property* properties[] = { VARIANT };
    return new BlockStateDefinition(this, properties, 1);
}

int PlanksBlock::getDescriptionId(int auxValue)
{
    return Variant::byData(auxValue)->getTranslationKey();
}

TextureAtlasSprite* PlanksBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return mIcons[state->getValue(VARIANT)->getData()];
}

void PlanksBlock::registerIcons(IconRegister* iconRegister)
{
    std::wstring base = getIconName();
    for (int i = 0; i < 6; ++i)
    {
        mIcons[i] = iconRegister->registerIcon(base + L"_" + gPlankVariants[i]->getSerializedName());
    }
}
