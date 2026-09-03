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

static PlanksBlock::Variant* gPlankVariants[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
static EnumProperty<PlanksBlock::Variant*>* gPlankVariantProperty = NULL;

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

std::wstring PlanksBlock::Variant::getName()
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
    gPlankVariants[0] = new Variant(0, L"oak", 0x13035FA5, MaterialColor::WOOD);
    gPlankVariants[1] = new Variant(1, L"spruce", 0x7A4DF272, MaterialColor::PODZOL);
    gPlankVariants[2] = new Variant(2, L"birch", 0x9BC6104A, MaterialColor::SAND);
    gPlankVariants[3] = new Variant(3, L"jungle", 0x8CE369FC, MaterialColor::DIRT);
    gPlankVariants[4] = new Variant(4, L"acacia", 0x674A2380, MaterialColor::COLOR_ORANGE);
    gPlankVariants[5] = new Variant(5, L"dark_oak", 0x0458BC59, MaterialColor::COLOR_BROWN);

    boost::unordered_set<Variant*> values;
    values.insert(gPlankVariants[0]);
    values.insert(gPlankVariants[1]);
    values.insert(gPlankVariants[2]);
    values.insert(gPlankVariants[3]);
    values.insert(gPlankVariants[4]);
    values.insert(gPlankVariants[5]);
    PlanksVariantPredicate predicate;
    gPlankVariantProperty = EnumProperty<Variant*>::create(L"variant", typeid(Variant), values, &predicate);
}

PlanksBlock::PlanksBlock()
    : Block(Material::WOOD)
{
    DerivedInit();
    TypedBoxed<Variant*> value(gPlankVariants[0]);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(gPlankVariantProperty, &value));
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
    return state->getValue(gPlankVariantProperty)->getData();
}

const BlockState* PlanksBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> value(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(gPlankVariantProperty, &value);
}

const MaterialColor* PlanksBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return state->getValue(gPlankVariantProperty)->getMaterialColor();
}

int PlanksBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gPlankVariantProperty)->getData();
}

BlockStateDefinition* PlanksBlock::createBlockStateDefinition()
{
    const Property* properties[] = { gPlankVariantProperty };
    return new BlockStateDefinition(this, properties, 1);
}

int PlanksBlock::getDescriptionId(int auxValue)
{
    return Variant::byData(auxValue)->getTranslationKey();
}

TextureAtlasSprite* PlanksBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return mIcons[state->getValue(gPlankVariantProperty)->getData()];
}

void PlanksBlock::registerIcons(IconRegister* iconRegister)
{
    std::wstring base = getIconName();
    for (int i = 0; i < 6; ++i)
    {
        mIcons[i] = iconRegister->registerIcon(base + L"_" + gPlankVariants[i]->getSerializedName());
    }
}
