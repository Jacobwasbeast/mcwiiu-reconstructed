#include "game/world/block/RedSandStoneBlock.hpp"
#include "game/core/StringIds.hpp"

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
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

RedSandStoneBlock::Variant* RedSandStoneBlock::Variant::DEFAULT =
    new RedSandStoneBlock::Variant(0, L"red_sandstone", IDS_TILE_REDSANDSTONE);
RedSandStoneBlock::Variant* RedSandStoneBlock::Variant::CHISELED =
    new RedSandStoneBlock::Variant(1, L"chiseled_red_sandstone", 0xD03F0851);
RedSandStoneBlock::Variant* RedSandStoneBlock::Variant::SMOOTH =
    new RedSandStoneBlock::Variant(2, L"smooth_red_sandstone", 0x40496822);
static RedSandStoneBlock::Variant* gRedSandStoneVariants[3] = {
    RedSandStoneBlock::Variant::DEFAULT,
    RedSandStoneBlock::Variant::CHISELED,
    RedSandStoneBlock::Variant::SMOOTH
};
EnumProperty<RedSandStoneBlock::Variant*>* RedSandStoneBlock::TYPE = NULL;

static Predicate<RedSandStoneBlock::Variant*>* gRedSandStoneTypePredicate =
    new Predicates<RedSandStoneBlock::Variant*>::ConstantPredicate(true);
static std::wstring gRedSandStoneTopTexture = L"red_sandstone_top";
static std::wstring gRedSandStoneBottomTexture = L"red_sandstone_bottom";
static std::wstring gRedSandStoneNormalTexture = L"red_sandstone_normal";
static std::wstring gRedSandStoneCarvedTexture = L"red_sandstone_carved";
static std::wstring gRedSandStoneSmoothTexture = L"red_sandstone_smooth";

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
    if (data < 0 || data > 2)
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
    std::wstring name = L"type";
    EnumProperty<Variant*>* type;
    {
        boost::unordered_set<Variant*> values;
        values.insert(gRedSandStoneVariants[0]);
        values.insert(gRedSandStoneVariants[1]);
        values.insert(gRedSandStoneVariants[2]);
        type = EnumProperty<Variant*>::create(
            name, typeid(Variant), values, gRedSandStoneTypePredicate);
    }
    TYPE = type;
}

RedSandStoneBlock::RedSandStoneBlock()
    : Block(Material::STONE, SandBlock::Variant::RED_SAND->getMaterialColor())
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(TYPE, Variant::DEFAULT));
    memset(mTextures, 0, sizeof(mTextures));
}

int RedSandStoneBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(TYPE)->getData();
}

const BlockState* RedSandStoneBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(TYPE, Variant::byData(data));
}

int RedSandStoneBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(TYPE)->getData();
}

BlockStateDefinition* RedSandStoneBlock::createBlockStateDefinition()
{
    static const Property* properties[1] = { TYPE };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* RedSandStoneBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Variant* variant = state->getValue(TYPE);
    if (direction != Direction::UP && direction == Direction::DOWN &&
        variant != Variant::CHISELED && variant != Variant::SMOOTH)
    {
        return mBottomTexture;
    }
    if (direction == Direction::UP || direction == Direction::DOWN)
    {
        return mTopTexture;
    }
    return mTextures[variant->getData()];
}

void RedSandStoneBlock::registerIcons(IconRegister* iconRegister)
{
    mTextures[0] = iconRegister->registerIcon(gRedSandStoneNormalTexture);
    mTextures[1] = iconRegister->registerIcon(gRedSandStoneCarvedTexture);
    mTextures[2] = iconRegister->registerIcon(gRedSandStoneSmoothTexture);
    mTopTexture = iconRegister->registerIcon(gRedSandStoneTopTexture);
    mBottomTexture = iconRegister->registerIcon(gRedSandStoneBottomTexture);
}
