#include "game/world/block/RandomizableLockableContainerBlockEntity.hpp"

#include "game/core/LootTable.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/core/LootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Container.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

RandomizableLockableContainerBlockEntity::RandomizableLockableContainerBlockEntity()
    : LockableContainerBlockEntity()
    , mLootTable(NULL)
    , mLootTableSeed(0)
    , mCustomName(new CensoredString())
{
}

RandomizableLockableContainerBlockEntity::~RandomizableLockableContainerBlockEntity()
{
}

ResourceLocation* RandomizableLockableContainerBlockEntity::getLootTable()
{
    return mLootTable;
}

long long RandomizableLockableContainerBlockEntity::getLootTableSeed()
{
    return mLootTableSeed;
}

bool RandomizableLockableContainerBlockEntity::hasCustomName()
{
    return !mCustomName->raw().empty() && mCustomName->checkParentalFilter() != 0;
}

not_null_ptr<ItemInstance> RandomizableLockableContainerBlockEntity::getItem(unsigned int slot)
{
    boost::shared_ptr<Player> player;
    unpackLootTable(player);
    return getItems()[slot];
}

not_null_ptr<ItemInstance> RandomizableLockableContainerBlockEntity::removeItem(unsigned int slot, int count)
{
    boost::shared_ptr<Player> player;
    unpackLootTable(player);
    not_null_ptr<ItemInstance> item =
        ContainerHelper::removeItem(getItems(), slot, static_cast<unsigned int>(count));
    if (!item->isEmpty())
    {
        setChanged();
    }
    return item;
}

not_null_ptr<ItemInstance> RandomizableLockableContainerBlockEntity::removeItemNoUpdate(int slot)
{
    boost::shared_ptr<Player> player;
    unpackLootTable(player);
    return ContainerHelper::takeItem(getItems(), static_cast<unsigned int>(slot));
}

void RandomizableLockableContainerBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> itemInstance)
{
    boost::shared_ptr<Player> player;
    unpackLootTable(player);
    getItems()[slot] = itemInstance;
    if (itemInstance->getCount() > getMaxStackSize())
    {
        itemInstance->setCount(getMaxStackSize());
    }
    setChanged();
}

bool RandomizableLockableContainerBlockEntity::stillValid(boost::shared_ptr<Player> player)
{
    if (mLevel->getBlockEntity(mPos) != shared_from_this())
    {
        return false;
    }
    return player->distanceAsIfHitscanned(mPos, *player->mBoundingBox) <= 30.25;
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
    boost::shared_ptr<Player> player;
    unpackLootTable(player);
    NonNullList<not_null_ptr<ItemInstance> > items = getItems();
    not_null_ptr<ItemInstance> empty;
    for (unsigned int slot = 0; slot < items.size(); ++slot)
    {
        items[slot] = empty;
    }
}

InteractionObject::NameData RandomizableLockableContainerBlockEntity::GetNetworkNameData()
{
    return InteractionObject::NameData(mCustomName->raw(), mCustomName->getUUID(),
                                       mCustomName->isUUIDLocal(), mCustomName->isChatCensored());
}

void RandomizableLockableContainerBlockEntity::unpackLootTable(const boost::shared_ptr<Player>& player)
{
    if (mLootTable == NULL)
    {
        return;
    }
    if (mLevel == NULL)
    {
        return;
    }

    LootTable* lootTable =
        static_cast<LootTables*>(mLevel->getLootTables())->get(mLootTable);
    if (lootTable == NULL)
    {
        return;
    }
    mLootTable = NULL;

    Random* random = mLootTableSeed == 0 ? new Random() : new Random(mLootTableSeed);

    float luck = 0.0f;
    if (player)
    {
        luck = player->getLuck();
    }

    LootTableContext context(luck, static_cast<ServerLevel*>(mLevel),
                             static_cast<LootTables*>(mLevel->getLootTables()),
                             player, boost::shared_ptr<Player>(), NULL);
    lootTable->fill(this, random, context);
    delete random;
}

void RandomizableLockableContainerBlockEntity::setLootTable(ResourceLocation* lootTable, long long seed)
{
    mLootTable = lootTable;
    mLootTableSeed = seed;
}

bool RandomizableLockableContainerBlockEntity::tryLoadLootTable(CompoundTag* tag)
{
    if (tag != NULL && tag->contains(std::wstring(L"LootTable"), 8))
    {
        mLootTable = new ResourceLocation(tag->getString(std::wstring(L"LootTable")));
        mLootTableSeed = tag->getLong(std::wstring(L"LootTableSeed"));
        return true;
    }
    return false;
}

bool RandomizableLockableContainerBlockEntity::trySaveLootTable(CompoundTag* tag)
{
    if (tag != NULL && mLootTable != NULL)
    {
        tag->putString(std::wstring(L"LootTable"), mLootTable->toString());
        if (mLootTableSeed != 0)
        {
            tag->putLong(std::wstring(L"LootTableSeed"), mLootTableSeed);
        }
        return true;
    }
    return false;
}

void RandomizableLockableContainerBlockEntity::setCustomName(const std::wstring& name,
                                                             const std::wstring& uuid,
                                                             bool chatCensored,
                                                             bool uuidLocal)
{
    mCustomName->set(name, uuid, chatCensored, uuidLocal);
}

void RandomizableLockableContainerBlockEntity::operator=(const RandomizableLockableContainerBlockEntity& other)
{
    BlockEntity::operator=(other);
    mLockKeyCode = other.mLockKeyCode;
    mLootTable = other.mLootTable;
    mLootTableSeed = other.mLootTableSeed;
    mCustomName = other.mCustomName;
}
