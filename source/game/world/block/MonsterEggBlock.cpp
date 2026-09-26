#include "game/world/block/MonsterEggBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

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

MonsterEggBlock::Variant* MonsterEggBlock::Variant::STONE =
    new MonsterEggBlock::Variant(0, L"stone", IDS_TILE_STONE_SILVERFISH);
MonsterEggBlock::Variant* MonsterEggBlock::Variant::COBBLESTONE =
    new MonsterEggBlock::Variant(1, L"cobblestone", 0xD8B28C61);
MonsterEggBlock::Variant* MonsterEggBlock::Variant::STONEBRICK =
    new MonsterEggBlock::Variant(2, L"stone_brick", 0x097EADE2);
MonsterEggBlock::Variant* MonsterEggBlock::Variant::MOSSY_STONEBRICK =
    new MonsterEggBlock::Variant(3, L"mossy_brick", 0x12B53EDE);
MonsterEggBlock::Variant* MonsterEggBlock::Variant::CRACKED_STONEBRICK =
    new MonsterEggBlock::Variant(4, L"cracked_brick", 0x881C257C);
MonsterEggBlock::Variant* MonsterEggBlock::Variant::CHISELED_STONEBRICK =
    new MonsterEggBlock::Variant(5, L"chiseled_brick", 0x1DB92A74);
MonsterEggBlock::Variant* MonsterEggBlock::Variant::VALUES[6] = {
    MonsterEggBlock::Variant::STONE,
    MonsterEggBlock::Variant::COBBLESTONE,
    MonsterEggBlock::Variant::STONEBRICK,
    MonsterEggBlock::Variant::MOSSY_STONEBRICK,
    MonsterEggBlock::Variant::CRACKED_STONEBRICK,
    MonsterEggBlock::Variant::CHISELED_STONEBRICK
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
    return VALUES[data];
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
        if (VALUES[i]->getHostBlockState() == state)
        {
            return VALUES[i];
        }
    }
    return STONE;
}

void MonsterEggBlock::blockStaticCtor()
{
    boost::unordered_set<Variant*> values;
    for (int i = 0; i < 6; ++i)
    {
        values.insert(Variant::VALUES[i]);
    }
    VARIANT = EnumProperty<Variant*>::create(
        L"variant", typeid(Variant*), values, Predicates<Variant*>::alwaysTrue);
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
    if (!level->mIsClientSide && level->getGameRules()->getBoolean(GameRules::RULE_DOBLOCKDROPS))
    {
        if (level->countInstanceOf(EntityType::MONSTER, false, NULL, NULL) < 70
            && level->countInstanceOf(EntityType::SILVERFISH, true, NULL, NULL) < 15)
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
    if (variant == Variant::STONE)
    {
        return Blocks::stone->getTexture(direction);
    }
    if (variant == Variant::COBBLESTONE)
    {
        return Blocks::cobblestone->getTexture(direction);
    }
    if (variant == Variant::STONEBRICK)
    {
        return Blocks::stonebrick->getTexture(direction);
    }
    if (variant == Variant::MOSSY_STONEBRICK)
    {
        TypedBoxed<StoneBrickBlock::Variant*> type(StoneBrickBlock::Variant::byData(StoneBrickBlock::MOSSY_META));
        return Blocks::stonebrick->getTexture(
            direction, Blocks::stonebrick->defaultBlockState()->setBoxedValue(StoneBrickBlock::VARIANT, &type));
    }
    if (variant == Variant::CRACKED_STONEBRICK)
    {
        TypedBoxed<StoneBrickBlock::Variant*> type(StoneBrickBlock::Variant::byData(StoneBrickBlock::CRACKED_META));
        return Blocks::stonebrick->getTexture(
            direction, Blocks::stonebrick->defaultBlockState()->setBoxedValue(StoneBrickBlock::VARIANT, &type));
    }
    if (variant == Variant::CHISELED_STONEBRICK)
    {
        TypedBoxed<StoneBrickBlock::Variant*> type(StoneBrickBlock::Variant::byData(StoneBrickBlock::CHISELED_META));
        return Blocks::stonebrick->getTexture(
            direction, Blocks::stonebrick->defaultBlockState()->setBoxedValue(StoneBrickBlock::VARIANT, &type));
    }
    return NULL;
}

void MonsterEggBlock::registerIcons(IconRegister*)
{
}
