#include "game/world/block/HopperBlockEntity.hpp"

#include <algorithm>

#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Inventory.hpp"
#include "game/ui/HopperMenu.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"

HopperBlockEntity::HopperBlockEntity()
    : RandomizableLockableContainerBlockEntity(), Hopper(), mCooldown(-1), mTransferCooldown(0), mEnabled(true)
{
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(5, ItemInstance::EMPTY);
}

HopperBlockEntity::~HopperBlockEntity()
{
}

BlockEntity* HopperBlockEntity::create()
{
    return new HopperBlockEntity();
}

std::wstring HopperBlockEntity::getName()
{
    return mCustomName ? mCustomName->get() : std::wstring(L"container.hopper");
}

std::wstring HopperBlockEntity::getCustomName()
{
    return mCustomName ? mCustomName->get() : std::wstring();
}

unsigned int HopperBlockEntity::getContainerSize()
{
    return 5;
}

bool HopperBlockEntity::inventoryEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
        if (!mItems[i]->isEmpty())
            return false;
    return true;
}

bool HopperBlockEntity::isEmpty()
{
    return inventoryEmpty();
}

not_null_ptr<ItemInstance> HopperBlockEntity::removeItem(unsigned int slot, int count)
{
    if (slot >= mItems.size())
        return ItemInstance::EMPTY;
    not_null_ptr<ItemInstance> result = ContainerHelper::removeItem(mItems, slot, count);
    if (!result->isEmpty())
        setChanged();
    return result;
}

void HopperBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> itemInstance)
{
    if (slot >= mItems.size())
        return;
    if (itemInstance->getCount() > getMaxStackSize())
        itemInstance->setCount(getMaxStackSize());
    mItems[slot] = itemInstance;
    setChanged();
}

int HopperBlockEntity::getMaxStackSize()
{
    return 64;
}

void HopperBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

AbstractContainerMenu* HopperBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory,
                                                       boost::shared_ptr<Player> player)
{
    boost::shared_ptr<Container> self = boost::dynamic_pointer_cast<Container>(shared_from_this());
    return new HopperMenu(boost::static_pointer_cast<Container>(inventory), self, player);
}

int HopperBlockEntity::getTypeId(bool)
{
    return 9;
}

Level* HopperBlockEntity::getLevel()
{
    return BlockEntity::getLevel();
}

double HopperBlockEntity::getLevelX()
{
    return static_cast<double>(mPos.x) + 0.5;
}

double HopperBlockEntity::getLevelY()
{
    return static_cast<double>(mPos.y) + 0.5;
}

double HopperBlockEntity::getLevelZ()
{
    return static_cast<double>(mPos.z) + 0.5;
}

void HopperBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     (DataWalker*)new ItemListDataWalker(0x1e00205ULL, L"Items"));
}

boost::shared_ptr<BlockEntity> HopperBlockEntity::clone()
{
    boost::shared_ptr<HopperBlockEntity> result(new HopperBlockEntity());
    BlockEntity::clone(result);
    result->mItems = mItems;
    result->mCooldown = mCooldown;
    result->mTransferCooldown = mTransferCooldown;
    result->mEnabled = mEnabled;
    if (mCustomName)
    {
        result->mCustomName.reset(new CensoredString());
        result->mCustomName->set(mCustomName->raw(), std::wstring(), false, false);
    }
    return result;
}

void HopperBlockEntity::load(CompoundTag* tag)
{
    LockableContainerBlockEntity::load(tag);
    ContainerHelper::loadAllItems(tag, mItems);
    mCooldown = tag->getInt(L"TransferCooldown");
}

void HopperBlockEntity::save(CompoundTag* tag)
{
    LockableContainerBlockEntity::save(tag);
    ContainerHelper::saveAllItems(tag, mItems);
    tag->putInt(L"TransferCooldown", mCooldown);
}

void HopperBlockEntity::tick()
{
    if (mCooldown > 0)
        --mCooldown;
    if (!isOnCooldown() && mEnabled)
    {
        tryMoveItems();
        mCooldown = 8;
    }
}

bool HopperBlockEntity::inventoryFull()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
        if (mItems[i]->isEmpty() || mItems[i]->getCount() < std::min(mItems[i]->getMaxStackSize(), getMaxStackSize()))
            return false;
    return true;
}

std::vector<boost::shared_ptr<Entity> >* HopperBlockEntity::getItemsAtAndAbove(Level* level, double x, double y, double z)
{
    AABB* box = AABB::newTemp(x - 0.5, y, z - 0.5, x + 0.5, y + 1.5, z + 0.5);
    return level->getEntitiesOfClass(typeid(ItemEntity), box);
}

boost::shared_ptr<Container> HopperBlockEntity::getContainerAt(Level* level, double x, double y, double z)
{
    if (level == NULL)
        return boost::shared_ptr<Container>();
    BlockPos pos(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z));
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    return boost::dynamic_pointer_cast<Container>(entity);
}

boost::shared_ptr<Container> HopperBlockEntity::getSourceContainer(Hopper* hopper)
{
    return getContainerAt(hopper->getLevel(), hopper->getLevelX(), hopper->getLevelY() + 1.0, hopper->getLevelZ());
}

bool HopperBlockEntity::isEmptyContainer(boost::shared_ptr<Container> container, const Direction*)
{
    return !container || container->isEmpty();
}

bool HopperBlockEntity::canTakeItemFromContainer(Container* container, not_null_ptr<ItemInstance> item, int slot, const Direction*)
{
    return container != NULL && slot >= 0 && static_cast<unsigned int>(slot) < container->getContainerSize() &&
           item.get() != NULL && !item->isEmpty();
}

bool HopperBlockEntity::canPlaceItemInContainer(Container* container, not_null_ptr<ItemInstance> item, int slot, const Direction*)
{
    return container != NULL && item.get() != NULL && !item->isEmpty() && container->canPlaceItem(slot, item);
}

bool HopperBlockEntity::canMergeItems(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second)
{
    return first->getItem() == second->getItem() && first->getAuxValue() == second->getAuxValue() &&
           first->getCount() < first->getMaxStackSize() && ItemInstance::tagMatches(first, second);
}

bool HopperBlockEntity::tryMoveInItem(Container* container, not_null_ptr<ItemInstance> item, int slot, const Direction* direction)
{
    if (!canPlaceItemInContainer(container, item, slot, direction))
        return false;
    container->setItem(slot, item);
    return true;
}

not_null_ptr<ItemInstance> HopperBlockEntity::addItem(Container* source, Container* destination,
                                                       not_null_ptr<ItemInstance> item, const Direction* direction)
{
    if (destination == NULL)
        return item;
    for (unsigned int i = 0; i < destination->getContainerSize(); ++i)
    {
        if (!canPlaceItemInContainer(destination, item, i, direction))
            continue;
        not_null_ptr<ItemInstance> existing = destination->getItem(i);
        if (existing->isEmpty())
        {
            destination->setItem(i, item);
            destination->setChanged();
            if (source != NULL)
                source->setChanged();
            return ItemInstance::EMPTY;
        }
        if (canMergeItems(existing, item))
        {
            int amount = std::min(item->getCount(), existing->getMaxStackSize() - existing->getCount());
            existing->grow(amount);
            item->shrink(amount);
            destination->setChanged();
            if (source != NULL)
                source->setChanged();
            if (item->isEmpty())
                return ItemInstance::EMPTY;
        }
    }
    return item;
}

bool HopperBlockEntity::tryTakeInItemFromSlot(Hopper* hopper, Container* container, int slot, const Direction* direction)
{
    not_null_ptr<ItemInstance> item = container->getItem(slot);
    if (!canTakeItemFromContainer(container, item, slot, direction))
        return false;
    Container* destination = dynamic_cast<Container*>(hopper);
    if (destination == NULL)
        return false;
    for (unsigned int i = 0; i < destination->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> existing = destination->getItem(i);
        if (existing->isEmpty())
        {
            destination->setItem(i, item);
            break;
        }
        if (canMergeItems(existing, item))
        {
            int amount = std::min(item->getCount(), existing->getMaxStackSize() - existing->getCount());
            existing->grow(amount);
            item->shrink(amount);
            if (item->isEmpty())
                break;
        }
    }
    container->setItem(slot, item);
    return true;
}

bool HopperBlockEntity::addItem(Container*, Container* destination, const boost::shared_ptr<ItemEntity>& entity)
{
    if (entity.get() == NULL)
        return false;
    not_null_ptr<ItemInstance> remaining = addItem(NULL, destination, entity->getItem(), NULL);
    entity->setItem(remaining);
    return remaining->isEmpty();
}

bool HopperBlockEntity::suckInItems(Hopper* hopper)
{
    boost::shared_ptr<Container> source = getSourceContainer(hopper);
    if (source)
    {
        for (unsigned int i = 0; i < source->getContainerSize(); ++i)
            if (tryTakeInItemFromSlot(hopper, source.get(), i, Direction::DOWN))
                return true;
    }
    return false;
}

bool HopperBlockEntity::tryMoveItems()
{
    return suckInItems(this) || ejectItems();
}

bool HopperBlockEntity::isFullContainer(boost::shared_ptr<Container> container, const Direction*)
{
    return !container || container->isFull();
}

bool HopperBlockEntity::ejectItems()
{
    boost::shared_ptr<Container> destination = getAttachedContainer();
    if (!destination)
        return false;
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        not_null_ptr<ItemInstance> item = mItems[i];
        if (item->isEmpty())
            continue;
        not_null_ptr<ItemInstance> remaining = addItem(this, destination.get(), item, NULL);
        if (remaining->isEmpty())
        {
            mItems[i] = remaining;
            setChanged();
            return true;
        }
        mItems[i] = remaining;
    }
    return false;
}

boost::shared_ptr<Container> HopperBlockEntity::getAttachedContainer()
{
    return getContainerAt(getLevel(), getLevelX(), getLevelY(), getLevelZ());
}

void HopperBlockEntity::setCooldown(int cooldown)
{
    mCooldown = cooldown;
}

bool HopperBlockEntity::isOnCooldown()
{
    return mCooldown > 0;
}

bool HopperBlockEntity::isOnCustomCooldown()
{
    return mCooldown > 8;
}

NonNullList<not_null_ptr<ItemInstance> > HopperBlockEntity::getItems()
{
    return mItems;
}

void HopperBlockEntity::setItems(NonNullList<not_null_ptr<ItemInstance> > items)
{
    mItems = items;
}

std::wstring HopperBlockEntity::getDisplayName()
{
    return getName();
}

unsigned long long HopperBlockEntity::GetType()
{
    return 0x01e0020500000000ULL;
}
