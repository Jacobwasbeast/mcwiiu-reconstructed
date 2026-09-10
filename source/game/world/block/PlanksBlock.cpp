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
    new PlanksBlock::Variant(0, L"oak", 0x13035FA5, MaterialColor::WOOD);
PlanksBlock::Variant* PlanksBlock::Variant::SPRUCE =
    new PlanksBlock::Variant(1, L"spruce", 0x7A4DF272, MaterialColor::PODZOL);
PlanksBlock::Variant* PlanksBlock::Variant::BIRCH =
    new PlanksBlock::Variant(2, L"birch", 0x9BC6104A, MaterialColor::SAND);
PlanksBlock::Variant* PlanksBlock::Variant::JUNGLE =
    new PlanksBlock::Variant(3, L"jungle", 0x8CE369FC, MaterialColor::DIRT);
PlanksBlock::Variant* PlanksBlock::Variant::ACACIA =
    new PlanksBlock::Variant(4, L"acacia", 0x674A2380, MaterialColor::COLOR_ORANGE);
PlanksBlock::Variant* PlanksBlock::Variant::DARK_OAK =
    new PlanksBlock::Variant(5, L"dark_oak", 0x0458BC59, MaterialColor::COLOR_BROWN);

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
    if (data > 5 || data < 0)
    {
        data = 0;
    }
    return gPlankVariants[data];
}

void PlanksBlock::blockStaticCtor()
{
    boost::unordered_set<Variant*> values;
    values.insert(gPlankVariants[0]);
    values.insert(gPlankVariants[1]);
    values.insert(gPlankVariants[2]);
    values.insert(gPlankVariants[3]);
    values.insert(gPlankVariants[4]);
    values.insert(gPlankVariants[5]);
    PlanksVariantPredicate predicate;
    VARIANT = EnumProperty<Variant*>::create(L"variant", typeid(Variant), values, &predicate);
}

PlanksBlock::PlanksBlock()
    : Block(Material::WOOD)
{
    DerivedInit();
    TypedBoxed<Variant*> value(gPlankVariants[0]);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(VARIANT, &value));
    for (int i = 0; i < 6; ++i)
    {
        mIcons[i] = NULL;
    }
}

PlanksBlock::~PlanksBlock()
{
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
