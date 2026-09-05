#include "game/item/MinecartContainer.hpp"

#include "game/core/L10N.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/Containers.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"

MinecartContainer::MinecartContainer(Level* level, double x, double y, double z)
    : Minecart(level, x, y, z), LockableContainer(), mItems(), mDropContentsWhenDead(true),
      mLootTable(NULL), mLootTableSeed(0)
{
    _init();
}

MinecartContainer::MinecartContainer(Level* level)
    : Minecart(level), LockableContainer(), mItems(), mDropContentsWhenDead(true),
      mLootTable(NULL), mLootTableSeed(0)
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
    if (slot >= mItems.size())
    {
        return ItemInstance::EMPTY;
    }
    return mItems[slot];
}

not_null_ptr<ItemInstance> MinecartContainer::removeItem(unsigned int slot, int count)
{
    not_null_ptr<ItemInstance> result = ContainerHelper::removeItem(mItems, slot, (unsigned int)count);
    if (!result->isEmpty())
    {
        setChanged();
    }
    return result;
}

not_null_ptr<ItemInstance> MinecartContainer::removeItemNoUpdate(int slot)
{
    if (slot < 0 || slot >= (int)mItems.size())
    {
        return ItemInstance::EMPTY;
    }
    not_null_ptr<ItemInstance> result = mItems[(unsigned int)slot];
    mItems[(unsigned int)slot] = ItemInstance::EMPTY;
    return result;
}

void MinecartContainer::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    if (slot >= mItems.size())
    {
        return;
    }
    mItems[slot] = item;
    if (!item->isEmpty() && item->getCount() > getMaxStackSize())
    {
        item->setCount(getMaxStackSize());
    }
    setChanged();
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
    return !mRemoved;
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
    return NULL;
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
    if (mDropContentsWhenDead)
    {
        clearContent();
    }
}

void MinecartContainer::changeDimension(int dimension)
{
    mDropContentsWhenDead = false;
    Entity::changeDimension(dimension);
}

void MinecartContainer::remove()
{
    if (mDropContentsWhenDead)
    {
        clearContent();
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
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(mItems.size(), ItemInstance::EMPTY);
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
    if (mRemoved)
    {
        return true;
    }
    return true;
}

void MinecartContainer::applyNaturalSlowdown()
{
    Minecart::applyNaturalSlowdown();
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
