#include "game/item/MinecartContainer.hpp"

#include <boost/pointer_cast.hpp>

#include "game/core/BuiltInLootTables.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/L10N.hpp"
#include "game/core/LootTable.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/Containers.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"

MinecartContainer::MinecartContainer(Level* level, double x, double y, double z)
    : Minecart(level, x, y, z), LockableContainer(), mItems()
{
    _init();
}

MinecartContainer::MinecartContainer(Level* level)
    : Minecart(level), LockableContainer(), mItems()
{
    _init();
}

MinecartContainer::~MinecartContainer()
{
}

void MinecartContainer::_init()
{
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(36, ItemInstance::EMPTY);
    mDropContentsWhenDead = true;
    mLootTable = NULL;
    mLootTableSeed = 0;
}

std::wstring MinecartContainer::getName()
{
    if (hasCustomName())
    {
        return getCustomName();
    }
    return std::wstring(L10N::GetString(0x917ea687));
}

std::wstring MinecartContainer::getCustomName()
{
    return Entity::getCustomName();
}

bool MinecartContainer::hasCustomName()
{
    return Entity::hasCustomName();
}

std::wstring MinecartContainer::getDisplayName()
{
    return Entity::getDisplayName();
}

void MinecartContainer::unpackLootTable(const boost::shared_ptr<Player>& player)
{
    if (mLootTable == NULL)
    {
        return;
    }

    LootTable* lootTable = BuiltInLootTables::getLootTable(mLootTable);
    if (lootTable == NULL)
    {
        return;
    }

    mLootTable = NULL;
    Random* random = mLootTableSeed == 0 ? new Random() : new Random(mLootTableSeed);
    boost::shared_ptr<Entity> entity = shared_from_this();
    LootTableContext context(player ? (float)player->getLuck() : 0.0f,
                             (ServerLevel*)mLevel,
                             (LootTables*)mLevel->getLootTables(),
                             entity,
                             player,
                             NULL);
    lootTable->fill(static_cast<Container*>(this), random, context);
}

bool MinecartContainer::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        if (!mItems[i]->isEmpty())
        {
            return false;
        }
    }
    return true;
}

not_null_ptr<ItemInstance> MinecartContainer::getItem(unsigned int slot)
{
    boost::shared_ptr<Player> player;
    unpackLootTable(player);
    return mItems[slot];
}

not_null_ptr<ItemInstance> MinecartContainer::removeItem(unsigned int slot, int count)
{
    boost::shared_ptr<Player> player;
    unpackLootTable(player);
    return ContainerHelper::removeItem(mItems, slot, count);
}

not_null_ptr<ItemInstance> MinecartContainer::removeItemNoUpdate(int slot)
{
    boost::shared_ptr<Player> player;
    unpackLootTable(player);
    not_null_ptr<ItemInstance> item = mItems[slot];
    if (item->isEmpty())
    {
        return ItemInstance::EMPTY;
    }
    mItems[slot] = ItemInstance::EMPTY;
    return item;
}

void MinecartContainer::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    boost::shared_ptr<Player> player;
    unpackLootTable(player);
    mItems[slot] = item;
    if (!item->isEmpty() && item->getCount() > getMaxStackSize())
    {
        item->setCount(getMaxStackSize());
    }
}

int MinecartContainer::getMaxStackSize()
{
    return 64;
}

void MinecartContainer::setChanged()
{
}

bool MinecartContainer::stillValid(boost::shared_ptr<Player> player)
{
    if (mRemoved)
    {
        return false;
    }
    BlockPos position(mX, mY, mZ);
    return player->distanceAsIfHitscanned(position, *player->mBoundingBox) <= 30.25;
}

void MinecartContainer::startOpen(boost::shared_ptr<Player> player)
{
}

void MinecartContainer::stopOpen(boost::shared_ptr<Player> player)
{
}

bool MinecartContainer::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    return true;
}

int MinecartContainer::getContainerData(int id)
{
    return 0;
}

void MinecartContainer::setContainerData(int id, int value)
{
}

int MinecartContainer::getContainerDataCount()
{
    return 0;
}

void MinecartContainer::clearContent()
{
    boost::shared_ptr<Player> player;
    unpackLootTable(player);
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        mItems[i] = ItemInstance::EMPTY;
    }
}

bool MinecartContainer::isLocked()
{
    return false;
}

void MinecartContainer::setLocked(LockCode* lockCode)
{
}

LockCode* MinecartContainer::getLockKeyCode()
{
    return LockCode::NO_LOCK;
}

void MinecartContainer::unlock()
{
}

ResourceLocation* MinecartContainer::getLootTable()
{
    return mLootTable;
}

long long MinecartContainer::getLootTableSeed()
{
    return mLootTableSeed;
}

void MinecartContainer::addDataWalkers(DataFixerUpper* fixer, unsigned long long gameTime)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity, (DataWalker*)new ItemListDataWalker(gameTime, L"Items"));
}

void MinecartContainer::destroy(const DamageSource* source)
{
    Minecart::destroy(source);
    if (mLevel->getGameRules()->getBoolean(6))
    {
        boost::shared_ptr<Entity> entity = shared_from_this();
        boost::shared_ptr<MinecartContainer> minecart = boost::static_pointer_cast<MinecartContainer>(entity);
        Containers::dropContents(mLevel, entity, boost::static_pointer_cast<Container>(minecart));
    }
}

boost::shared_ptr<Entity> MinecartContainer::changeDimension(int dimension)
{
    mDropContentsWhenDead = false;
    return Entity::changeDimension(dimension);
}

void MinecartContainer::remove()
{
    if (mDropContentsWhenDead)
    {
        boost::shared_ptr<Entity> entity = shared_from_this();
        boost::shared_ptr<MinecartContainer> minecart = boost::static_pointer_cast<MinecartContainer>(entity);
        Containers::dropContents(mLevel, entity, boost::static_pointer_cast<Container>(minecart));
    }
    Entity::remove();
}

void MinecartContainer::addAdditonalSaveData(CompoundTag* tag)
{
    Minecart::addAdditonalSaveData(tag);
    if (mLootTable == NULL || StructureFeatureIO::getVersion() < 2)
    {
        ContainerHelper::saveAllItems(tag, mItems);
    }
    else
    {
        tag->putString(L"LootTable", mLootTable->toString());
        if (mLootTableSeed != 0)
        {
            tag->putLong(L"LootTableSeed", mLootTableSeed);
        }
    }
}

void MinecartContainer::readAdditionalSaveData(CompoundTag* tag)
{
    Minecart::readAdditionalSaveData(tag);
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
    if (tag->contains(L"LootTable", 8) && StructureFeatureIO::getVersion() > 1)
    {
        mLootTable = new ResourceLocation(tag->getString(L"LootTable"));
        mLootTableSeed = tag->getLong(L"LootTableSeed");
    }
    else
    {
        ContainerHelper::loadAllItems(tag, mItems);
    }
}

bool MinecartContainer::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    if (!player->isAllowedToInteract(shared_from_this()))
    {
        return false;
    }

    if (!mLevel->mIsClientSide)
    {
        boost::shared_ptr<MinecartContainer> minecart = boost::static_pointer_cast<MinecartContainer>(shared_from_this());
        player->openContainer(boost::static_pointer_cast<Container>(minecart));
    }
    return true;
}

void MinecartContainer::applyNaturalSlowdown()
{
    float slowdown = 0.98f;
    if (mLootTable == NULL)
    {
        boost::shared_ptr<MinecartContainer> minecart = boost::static_pointer_cast<MinecartContainer>(shared_from_this());
        int signal = AbstractContainerMenu::getRedstoneSignalFromContainer(
            boost::static_pointer_cast<Container>(minecart));
        slowdown += (15 - signal) * 0.001f;
    }
    mXd *= slowdown;
    mYd *= 0.0;
    mZd *= slowdown;
}

void MinecartContainer::setDropContainerContent(bool dropContents)
{
    mDropContentsWhenDead = dropContents;
}

void MinecartContainer::setLootTable(ResourceLocation* lootTable, long long seed)
{
    mLootTable = lootTable;
    mLootTableSeed = seed;
}

unsigned int MinecartContainer::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_bAllowedToInteract ? 0x41590ad1 : 0xffffffff;
}
