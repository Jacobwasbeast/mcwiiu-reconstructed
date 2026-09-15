#include "game/world/block/MonsterEggBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Silverfish.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/core/GameRules.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/StoneBrickBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Material.hpp"

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
EnumProperty<MonsterEggBlock::Variant*>* MonsterEggBlock::VARIANT = NULL;

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
    if (VARIANT == NULL)
    {
        boost::unordered_set<Variant*> values;
        for (int i = 0; i < 6; ++i)
        {
            values.insert(gMonsterEggVariants[i]);
        }
        VARIANT = EnumProperty<Variant*>::create(
            L"variant", typeid(Variant*), values, Predicates<Variant*>::alwaysTrue);
    }
}

MonsterEggBlock::MonsterEggBlock()
    : Block(Material::CLAY)
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
    int data = state->getValue(VARIANT)->getData();
    if (data > 2)
    {
        return not_null_ptr<ItemInstance>(new ItemInstance(
            Blocks::stonebrick, 1, StoneBrickBlock::Variant::byData(data - 2)->getData()));
    }
    if (data == 1)
    {
        return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::cobblestone));
    }
    if (data == 2)
    {
        return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stonebrick));
    }
    return not_null_ptr<ItemInstance>(new ItemInstance(Blocks::stone));
}

void MonsterEggBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState*, float, int)
{
    if (!level->mIsClientSide && level->getGameRules()->getBoolean(5))
    {
        if (level->countInstanceOf(ENTITY_TYPE_MONSTER, false, NULL, NULL) < 70
            && level->countInstanceOf(0x000000005D540044ULL, true, NULL, NULL) < 15)
        {
            boost::shared_ptr<Silverfish> silverfish(new Silverfish(level));
            silverfish->moveTo(pos.x + 0.5, pos.y, pos.z + 0.5, 0.0f, 0.0f);
            level->addEntity(silverfish);
            silverfish->spawnAnim();
        }
    }
}

not_null_ptr<ItemInstance> MonsterEggBlock::getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(
        this, 1, state->getBlock()->convertBlockStateToLegacyData(state)));
}

const BlockState* MonsterEggBlock::getBlockState(int data)
{
    TypedBoxed<Variant*> value(Variant::byData(data));
    return defaultBlockState()->setBoxedValue(VARIANT, &value);
}

int MonsterEggBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

BlockStateDefinition* MonsterEggBlock::createBlockStateDefinition()
{
    const Property* properties[] = { VARIANT };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* MonsterEggBlock::getTexture(const Direction* direction, const BlockState* state)
{
    Variant* variant = state->getValue(VARIANT);
    if (variant == &gMonsterEggVariant0)
    {
        return Blocks::stone->getTexture(direction);
    }
    if (variant == &gMonsterEggVariant1)
    {
        return Blocks::cobblestone->getTexture(direction);
    }
    if (variant == &gMonsterEggVariant2)
    {
        return Blocks::stonebrick->getTexture(direction);
    }
    return Blocks::stonebrick->getTexture(direction, Blocks::stonebrick->getBlockState(variant->getData() - 2));
}

void MonsterEggBlock::registerIcons(IconRegister*)
{
}
