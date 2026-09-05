#include "game/world/block/MonsterEggBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

static MonsterEggBlock::Variant gMonsterEggVariant0(0, L"stone", 0);
static MonsterEggBlock::Variant gMonsterEggVariant1(1, L"cobblestone", 1);
static MonsterEggBlock::Variant gMonsterEggVariant2(2, L"stone_brick", 2);
static MonsterEggBlock::Variant gMonsterEggVariant3(3, L"mossy_stone_brick", 3);
static MonsterEggBlock::Variant gMonsterEggVariant4(4, L"cracked_stone_brick", 4);
static MonsterEggBlock::Variant gMonsterEggVariant5(5, L"chiseled_stone_brick", 5);
static MonsterEggBlock::Variant* gMonsterEggVariants[6] = {
    &gMonsterEggVariant0, &gMonsterEggVariant1, &gMonsterEggVariant2,
    &gMonsterEggVariant3, &gMonsterEggVariant4, &gMonsterEggVariant5
};
static EnumProperty<MonsterEggBlock::Variant*>* gMonsterEggVariantProperty = NULL;

MonsterEggBlock::Variant::Variant(int data, const std::wstring& name, int translationKey)
    : mData(data), mName(name), mTranslationKey(translationKey)
{
}

std::wstring MonsterEggBlock::Variant::toString() const
{
    return mName;
}

std::wstring MonsterEggBlock::Variant::getSerializedName() const
{
    return mName;
}

int MonsterEggBlock::Variant::getData()
{
    return mData;
}

int MonsterEggBlock::Variant::getTranslationKey()
{
    return mTranslationKey;
}

MonsterEggBlock::Variant* MonsterEggBlock::Variant::byData(int data)
{
    if (data > 5)
    {
        data = 0;
    }
    return gMonsterEggVariants[data];
}

const BlockState* MonsterEggBlock::Variant::getHostBlockState()
{
    if (mData == 1)
    {
        return Blocks::cobblestone->defaultBlockState();
    }
    if (mData >= 2 && mData <= 5)
    {
        return Blocks::stonebrick->getBlockState(mData - 2);
    }
    return Blocks::stone->defaultBlockState();
}

MonsterEggBlock::Variant* MonsterEggBlock::Variant::byHostBlock(const BlockState* state)
{
    for (int i = 0; i < 6; ++i)
    {
        if (gMonsterEggVariants[i]->getHostBlockState() == state)
        {
            return gMonsterEggVariants[i];
        }
    }
    return &gMonsterEggVariant0;
}

void MonsterEggBlock::blockStaticCtor()
{
    if (gMonsterEggVariantProperty == NULL)
    {
        boost::unordered_set<Variant*> values;
        for (int i = 0; i < 6; ++i)
        {
            values.insert(gMonsterEggVariants[i]);
        }
        gMonsterEggVariantProperty = EnumProperty<Variant*>::create(
            L"variant", typeid(Variant*), values, Predicates<Variant*>::alwaysTrue);
    }
}

MonsterEggBlock::MonsterEggBlock()
    : Block(NULL)
{
    DerivedInit();
    setDestroyTime(4.5f);
}

MonsterEggBlock::~MonsterEggBlock()
{
}

int MonsterEggBlock::getResourceCount(Random*)
{
    return 0;
}

bool MonsterEggBlock::isCompatibleHostBlock(const BlockState* state)
{
    return state == Blocks::stone->defaultBlockState()
        || state->getBlock() == Blocks::cobblestone
        || state->getBlock() == Blocks::stonebrick;
}

not_null_ptr<ItemInstance> MonsterEggBlock::getSilkTouchItemInstance(const BlockState* state)
{
    Variant* variant = state->getValue(gMonsterEggVariantProperty);
    if (variant->getData() > 2)
    {
        return Block::getSilkTouchItemInstance(state);
    }
    return not_null_ptr<ItemInstance>();
}

void MonsterEggBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance, int lootBonus)
{
    if (level->mIsClientSide || !level->getGameRules()->getBoolean(5))
    {
        return;
    }
    Block::spawnResources(level, pos, state, chance, lootBonus);
}

not_null_ptr<ItemInstance> MonsterEggBlock::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state)
{
    return Block::getCloneItemInstance(level, pos, state);
}

const BlockState* MonsterEggBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> value(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(gMonsterEggVariantProperty, &value);
}

int MonsterEggBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gMonsterEggVariantProperty)->getData();
}

BlockStateDefinition* MonsterEggBlock::createBlockStateDefinition()
{
    const Property* properties[] = { gMonsterEggVariantProperty };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* MonsterEggBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Variant* variant = state->getValue(gMonsterEggVariantProperty);
    if (variant == &gMonsterEggVariant0)
    {
        return Blocks::stone->getTexture(direction);
    }
    if (variant == &gMonsterEggVariant1)
    {
        return Blocks::cobblestone->getTexture(direction);
    }
    return Blocks::stonebrick->getTexture(direction, Blocks::stonebrick->getBlockState(variant->getData() - 2));
}

void MonsterEggBlock::registerIcons(IconRegister*)
{
}
