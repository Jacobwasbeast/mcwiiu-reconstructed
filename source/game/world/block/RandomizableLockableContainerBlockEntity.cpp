#include "game/world/block/RandomizableLockableContainerBlockEntity.hpp"

#include <cwchar>

#include "game/data/CompoundTag.hpp"
#include "game/core/LootTable.hpp"
#include "game/core/LootTables.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"

class ServerLevel;

RandomizableLockableContainerBlockEntity::RandomizableLockableContainerBlockEntity()
    : LockableContainerBlockEntity()
    , mItems()
    , mLootTable(NULL)
    , mLootTableSeed(0)
    , mCustomName(new CensoredString())
{
}

RandomizableLockableContainerBlockEntity::~RandomizableLockableContainerBlockEntity()
{
}

bool RandomizableLockableContainerBlockEntity::hasCustomName()
{
    return mCustomName && !mCustomName->raw().empty() && mCustomName->checkParentalFilter();
}

void RandomizableLockableContainerBlockEntity::unpackLootTable(const boost::shared_ptr<Player>& player)
{
    if (mLootTable == NULL || getLevel() == NULL)
    {
        return;
    }
    LootTables* lootTables = static_cast<LootTables*>(getLevel()->getLootTables());
    LootTable* lootTable = lootTables == NULL ? NULL : lootTables->get(mLootTable);
    mLootTable = NULL;
    if (lootTable == NULL)
    {
        return;
    }
    Random random;
    if (mLootTableSeed != 0)
    {
        random = Random(mLootTableSeed);
    }
    boost::shared_ptr<Entity> thisEntity;
    LootTableContext context(0.0f, (ServerLevel*)getLevel(), lootTables,
                             thisEntity, player, NULL);
    lootTable->fill(this, &random, context);
}

not_null_ptr<ItemInstance> RandomizableLockableContainerBlockEntity::getItem(unsigned int slot)
{
    boost::shared_ptr<Player> player;
    unpackLootTable(player);
    if (slot >= mItems.size())
    {
        return not_null_ptr<ItemInstance>();
    }
    return mItems[slot];
}

not_null_ptr<ItemInstance> RandomizableLockableContainerBlockEntity::removeItem(unsigned int slot, int count)
{
    boost::shared_ptr<Player> player;
    unpackLootTable(player);
    if (slot >= mItems.size())
    {
        return not_null_ptr<ItemInstance>();
    }
    return ContainerHelper::removeItem(mItems, slot, static_cast<unsigned int>(count));
}

not_null_ptr<ItemInstance> RandomizableLockableContainerBlockEntity::removeItemNoUpdate(int slot)
{
    boost::shared_ptr<Player> player;
    unpackLootTable(player);
    if (slot < 0 || static_cast<unsigned int>(slot) >= mItems.size())
    {
        return not_null_ptr<ItemInstance>();
    }
    return ContainerHelper::takeItem(mItems, static_cast<unsigned int>(slot));
}

void RandomizableLockableContainerBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> itemInstance)
{
    boost::shared_ptr<Player> player;
    unpackLootTable(player);
    if (slot < mItems.size())
    {
        mItems[slot] = itemInstance;
    }
}

bool RandomizableLockableContainerBlockEntity::stillValid(boost::shared_ptr<Player> player)
{
    return player.get() != NULL && getLevel() != NULL;
}

void RandomizableLockableContainerBlockEntity::startOpen(boost::shared_ptr<Player> player)
{
}

void RandomizableLockableContainerBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
}

bool RandomizableLockableContainerBlockEntity::canPlaceItem(int slot, not_null_ptr<ItemInstance> itemInstance)
{
    return true;
}

int RandomizableLockableContainerBlockEntity::getContainerData(int id)
{
    return 0;
}

void RandomizableLockableContainerBlockEntity::setContainerData(int id, int value)
{
}

int RandomizableLockableContainerBlockEntity::getContainerDataCount()
{
    return 0;
}

void RandomizableLockableContainerBlockEntity::clearContent()
{
    for (unsigned int slot = 0; slot < mItems.size(); ++slot)
    {
        mItems[slot] = not_null_ptr<ItemInstance>();
    }
}

InteractionObject::NameData RandomizableLockableContainerBlockEntity::GetNetworkNameData()
{
    if (mCustomName)
    {
        return InteractionObject::NameData(mCustomName->raw(), L"", true, false);
    }
    return InteractionObject::NameData(std::wstring(), L"", true, false);
}

ResourceLocation* RandomizableLockableContainerBlockEntity::getLootTable()
{
    return mLootTable;
}

long long RandomizableLockableContainerBlockEntity::getLootTableSeed()
{
    return mLootTableSeed;
}

void RandomizableLockableContainerBlockEntity::setLootTable(ResourceLocation* lootTable, long long seed)
{
    mLootTable = lootTable;
    mLootTableSeed = seed;
}

bool RandomizableLockableContainerBlockEntity::tryLoadLootTable(CompoundTag* tag)
{
    if (tag == NULL)
    {
        return false;
    }
    const std::wstring lootTableKey(L"LootTable");
    if (!tag->contains(lootTableKey, 8))
    {
        return false;
    }
    mLootTable = new ResourceLocation(tag->getString(lootTableKey));
    mLootTableSeed = tag->getLong(std::wstring(L"LootTableSeed"));
    return true;
}

bool RandomizableLockableContainerBlockEntity::trySaveLootTable(CompoundTag* tag)
{
    if (tag == NULL || mLootTable == NULL)
    {
        return false;
    }
    tag->putString(std::wstring(L"LootTable"), mLootTable->toString());
    if (mLootTableSeed != 0)
    {
        tag->putLong(std::wstring(L"LootTableSeed"), mLootTableSeed);
    }
    return true;
}

void RandomizableLockableContainerBlockEntity::setCustomName(const std::wstring& name,
                                                               const std::wstring& uuid,
                                                               bool chatCensored,
                                                               bool uuidLocal)
{
    if (mCustomName)
    {
        mCustomName->set(name, uuid, chatCensored, uuidLocal);
    }
}

void RandomizableLockableContainerBlockEntity::operator=(const RandomizableLockableContainerBlockEntity& other)
{
    BlockEntity::operator=(other);
    mItems = other.mItems;
    mLootTable = other.mLootTable;
    mLootTableSeed = other.mLootTableSeed;
    mCustomName = other.mCustomName;
}
