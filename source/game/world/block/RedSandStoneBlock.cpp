#include "game/world/block/RedSandStoneBlock.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

RedSandStoneBlock::Variant* RedSandStoneBlock::Variant::DEFAULT =
    new RedSandStoneBlock::Variant(0, L"red_sandstone", 1891675551);
RedSandStoneBlock::Variant* RedSandStoneBlock::Variant::CHISELED =
    new RedSandStoneBlock::Variant(1, L"chiseled_red_sandstone", -801175471);
RedSandStoneBlock::Variant* RedSandStoneBlock::Variant::SMOOTH =
    new RedSandStoneBlock::Variant(2, L"smooth_red_sandstone", 1078552610);
static RedSandStoneBlock::Variant* gRedSandStoneVariants[3] = {
    RedSandStoneBlock::Variant::DEFAULT,
    RedSandStoneBlock::Variant::CHISELED,
    RedSandStoneBlock::Variant::SMOOTH
};
EnumProperty<RedSandStoneBlock::Variant*>* RedSandStoneBlock::TYPE = NULL;

RedSandStoneBlock::Variant::Variant(int data, const std::wstring& name, int translationKey)
    : mData(data), mName(name), mTranslationKey(translationKey)
{
}

int RedSandStoneBlock::Variant::getData()
{
    return mData;
}

RedSandStoneBlock::Variant* RedSandStoneBlock::Variant::byData(int data)
{
    if (data > 2)
    {
        data = 0;
    }
    return gRedSandStoneVariants[data];
}

std::wstring RedSandStoneBlock::Variant::toString() const
{
    return mName;
}

std::wstring RedSandStoneBlock::Variant::getSerializedName() const
{
    return mName;
}

int RedSandStoneBlock::Variant::getTranslationKey()
{
    return mTranslationKey;
}

void RedSandStoneBlock::blockStaticCtor()
{
    boost::unordered_set<Variant*> values;
    values.insert(gRedSandStoneVariants[0]);
    values.insert(gRedSandStoneVariants[1]);
    values.insert(gRedSandStoneVariants[2]);
    TYPE = EnumProperty<Variant*>::create(
        L"type", typeid(Variant*), values, Predicates<Variant*>::alwaysTrue);
}

RedSandStoneBlock::RedSandStoneBlock()
    : Block(Material::STONE, SandBlock::Variant::RED_SAND->getMaterialColor())
{
    DerivedInit();
    TypedBoxed<Variant*> value(Variant::DEFAULT);
    registerDefaultState(defaultBlockState()->setBoxedValue(TYPE, &value));
    for (int i = 0; i < 5; ++i)
    {
        mTextures[i] = NULL;
    }
}

RedSandStoneBlock::~RedSandStoneBlock()
{
}

int RedSandStoneBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(TYPE)->getData();
}

const BlockState* RedSandStoneBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> value(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(TYPE, &value);
}

int RedSandStoneBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(TYPE)->getData();
}

BlockStateDefinition* RedSandStoneBlock::createBlockStateDefinition()
{
    const Property* properties[1];
    properties[0] = TYPE;
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* RedSandStoneBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Variant* variant = state->getValue(TYPE);
    if (direction != Direction::UP && direction == Direction::DOWN &&
        variant != Variant::CHISELED && variant != Variant::SMOOTH)
    {
        return mTextures[4];
    }
    if (direction == Direction::UP || direction == Direction::DOWN)
    {
        return mTextures[3];
    }
    return mTextures[variant->getData()];
}

void RedSandStoneBlock::registerIcons(IconRegister* iconRegister)
{
    mTextures[0] = iconRegister->registerIcon(L"red_sandstone_normal");
    mTextures[1] = iconRegister->registerIcon(L"red_sandstone_carved");
    mTextures[2] = iconRegister->registerIcon(L"red_sandstone_smooth");
    mTextures[3] = iconRegister->registerIcon(L"red_sandstone_top");
    mTextures[4] = iconRegister->registerIcon(L"red_sandstone_bottom");
}
