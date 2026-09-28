#include "game/world/block/HopperBlockEntity.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>

#include "game/core/Mth.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/item/WorldlyContainer.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Inventory.hpp"
#include "game/ui/HopperMenu.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/HopperBlock.hpp"

HopperBlockEntity::HopperBlockEntity()
    : RandomizableLockableContainerBlockEntity(), Hopper(), mCooldown(-1), mTransferCooldown(0), mEnabled(true)
{
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(5, ItemInstance::EMPTY);
}

HopperBlockEntity::~HopperBlockEntity()
{
    delete[] mItems.items;
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
    unpackLootTable(boost::shared_ptr<Player>());
    return ContainerHelper::removeItem(getItems(), slot, count);
}

void HopperBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> itemInstance)
{
    unpackLootTable(boost::shared_ptr<Player>());
    getItems()[slot] = itemInstance;
    if (itemInstance->getCount() > getMaxStackSize())
        itemInstance->setCount(getMaxStackSize());
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
    unpackLootTable(player);
    boost::shared_ptr<Container> self = boost::dynamic_pointer_cast<Container>(shared_from_this());
    return new HopperMenu(boost::static_pointer_cast<Container>(inventory), self, player);
}

int HopperBlockEntity::getTypeId(bool)
{
    return ClientboundContainerOpenPacket::HOPPER;
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
                     (DataWalker*)new ItemListDataWalker(BlockEntityType::HOPPER, L"Items"));
}

boost::shared_ptr<BlockEntity> HopperBlockEntity::clone()
{
    boost::shared_ptr<HopperBlockEntity> result(new HopperBlockEntity());
    BlockEntity::clone(result);
    result->mCustomName->set(mCustomName->raw(), mCustomName->getUUID(), mCustomName->isChatCensored(), mCustomName->isUUIDLocal());
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        if (!mItems[i]->isEmpty())
            result->mItems[i] = mItems[i]->copy();
    }
    result->mEnabled = mEnabled;
    return result;
}

void HopperBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
    if (!tryLoadLootTable(tag))
        ContainerHelper::loadAllItems(tag, mItems);
    mCustomName->loadFromTag(tag);
    mCooldown = tag->getInt(L"TransferCooldown");
}

CompoundTag* HopperBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mCustomName->saveToTag(tag);
    if (!trySaveLootTable(tag))
    {
        ContainerHelper::saveAllItems(tag, mItems);
    }
    tag->putInt(L"TransferCooldown", mCooldown);
    return tag;
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
    BlockPos pos(Mth::floor(x), Mth::floor(y), Mth::floor(z));
    boost::shared_ptr<Container> container = boost::dynamic_pointer_cast<Container>(level->getBlockEntity(pos));
    if (container)
    {
        if (boost::dynamic_pointer_cast<ChestBlockEntity>(container))
        {
            ChestBlock* chestBlock = dynamic_cast<ChestBlock*>(level->getBlockState(pos)->getBlock());
            if (chestBlock != NULL)
                container = chestBlock->getContainer(level, pos, true);
        }
        if (container)
            return container;
    }
    AABB* box = AABB::newTemp(x - 0.5, y - 0.5, z - 0.5, x + 0.5, y + 0.5, z + 0.5);
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntities(boost::shared_ptr<Entity>(), box, NULL);
    if (entities != NULL && !entities->empty())
        container = boost::dynamic_pointer_cast<Container>(entities->at(level->mRandom->nextInt(static_cast<int>(entities->size()))));
    return container;
}

boost::shared_ptr<Container> HopperBlockEntity::getSourceContainer(Hopper* hopper)
{
    return getContainerAt(hopper->getLevel(), hopper->getLevelX(), hopper->getLevelY() + 1.0, hopper->getLevelZ());
}

bool HopperBlockEntity::isEmptyContainer(boost::shared_ptr<Container> container, const Direction* direction)
{
    boost::shared_ptr<WorldlyContainer> worldly = boost::dynamic_pointer_cast<WorldlyContainer>(container);
    if (!worldly)
    {
        unsigned int size = container->getContainerSize();
        for (unsigned int i = 0; i < size; ++i)
        {
            if (!container->getItem(i)->isEmpty())
                return false;
        }
    }
    else
    {
        arrayWithLength<int> slots = worldly->getSlotsForFace(direction);
        for (unsigned int i = 0; i < slots.length; ++i)
        {
            if (!worldly->getItem(slots.data[i])->isEmpty())
                return false;
        }
    }
    return true;
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

not_null_ptr<ItemInstance> HopperBlockEntity::tryMoveInItem(Container* source, Container* destination,
                                                              not_null_ptr<ItemInstance> item, int slot,
                                                              const Direction* direction)
{
    not_null_ptr<ItemInstance> existing = destination->getItem(slot);
    if (!canPlaceItemInContainer(destination, item, slot, direction))
        return item;
    if (!existing->isEmpty())
    {
        if (!canMergeItems(existing, item))
            return item;
        int amount = std::min(item->getMaxStackSize() - existing->getCount(), item->getCount());
        if (amount == 0)
            return item;
        item->shrink(amount);
        existing->grow(amount);
    }
    else
    {
        destination->setItem(slot, item);
        item = ItemInstance::EMPTY;
    }
    HopperBlockEntity* destinationHopper = dynamic_cast<HopperBlockEntity*>(destination);
    if (destinationHopper != NULL && !destinationHopper->isOnCustomCooldown())
    {
        bool sourceIsHopper = dynamic_cast<HopperBlockEntity*>(source) != NULL;
        destinationHopper->setCooldown(sourceIsHopper ? 7 : 8);
    }
    destination->setChanged();
    return item;
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
    if (item->isEmpty() || !canTakeItemFromContainer(container, item, slot, direction))
    {
        return false;
    }

    not_null_ptr<ItemInstance> original = item->copy();
    not_null_ptr<ItemInstance> remainder = addItem(container, hopper, container->removeItem(slot, 1), NULL);
    if (remainder->isEmpty())
    {
        container->setChanged();
        return true;
    }
    container->setItem(slot, original);
    return false;
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
    if (!source)
    {
        std::vector<boost::shared_ptr<Entity> >* entities = getItemsAtAndAbove(
            hopper->getLevel(), hopper->getLevelX(), hopper->getLevelY(), hopper->getLevelZ());
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
        {
            boost::shared_ptr<ItemEntity> itemEntity = boost::dynamic_pointer_cast<ItemEntity>(*it);
            if (addItem(NULL, dynamic_cast<Container*>(hopper), itemEntity))
            {
                delete entities;
                return true;
            }
        }
        delete entities;
        return false;
    }
    if (isEmptyContainer(source, Direction::DOWN))
        return false;
    boost::shared_ptr<WorldlyContainer> worldly = boost::dynamic_pointer_cast<WorldlyContainer>(source);
    if (worldly)
    {
        arrayWithLength<int> slots = worldly->getSlotsForFace(Direction::DOWN);
        for (unsigned int i = 0; i < slots.length; ++i)
            if (tryTakeInItemFromSlot(hopper, source.get(), slots.data[i], Direction::DOWN))
                return true;
    }
    else
    {
        for (unsigned int i = 0; i < source->getContainerSize(); ++i)
            if (tryTakeInItemFromSlot(hopper, source.get(), i, Direction::DOWN))
                return true;
    }
    return false;
}

bool HopperBlockEntity::tryMoveItems()
{
    if (mLevel == NULL || mLevel->mIsClientSide || isOnCooldown())
        return false;
    if (!HopperBlock::isTurnedOn(getData()))
        return false;
    bool ejected = false;
    if (!inventoryEmpty())
        ejected = ejectItems();
    bool moved = ejected;
    if (!inventoryFull())
    {
        bool sucked = suckInItems(this);
        moved = sucked || ejected;
    }
    if (moved)
    {
        setCooldown(8);
        setChanged();
        return true;
    }
    return false;
}

bool HopperBlockEntity::isFullContainer(boost::shared_ptr<Container> container, const Direction* direction)
{
    boost::shared_ptr<WorldlyContainer> worldly = boost::dynamic_pointer_cast<WorldlyContainer>(container);
    if (!worldly)
    {
        unsigned int size = container->getContainerSize();
        for (unsigned int i = 0; i < size; ++i)
        {
            not_null_ptr<ItemInstance> item = container->getItem(i);
            if (item->isEmpty() || item->getCount() != item->getMaxStackSize())
                return false;
        }
    }
    else
    {
        arrayWithLength<int> slots = worldly->getSlotsForFace(direction);
        for (unsigned int i = 0; i < slots.length; ++i)
        {
            not_null_ptr<ItemInstance> item = worldly->getItem(slots.data[i]);
            if (item->isEmpty() || item->getCount() != item->getMaxStackSize())
                return false;
        }
    }
    return true;
}

bool HopperBlockEntity::ejectItems()
{
    boost::shared_ptr<Container> destination = getAttachedContainer();
    if (!destination)
        return false;
    const Direction* face = HopperBlock::getAttachedFace(getData())->getOpposite();
    if (isFullContainer(destination, face))
        return false;
    for (unsigned int i = 0; i < getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = getItem(i);
        if (item->isEmpty())
            continue;
        not_null_ptr<ItemInstance> original = item->copy();
        not_null_ptr<ItemInstance> extracted = removeItem(i, 1);
        not_null_ptr<ItemInstance> remaining = addItem(this, destination.get(), extracted, face);
        if (remaining->isEmpty())
        {
            destination->setChanged();
            return true;
        }
        setItem(i, original);
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
    return BlockEntityType::HOPPER;
}
