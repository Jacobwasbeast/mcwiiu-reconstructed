#include "game/world/block/DispenserBlockEntity.hpp"

#include <boost/shared_ptr.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/core/CensoredString.hpp"
#include "game/core/DataWalker.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/ui/DispenserMenu.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockEntity.hpp"

static Random gDispenserRandom;
const unsigned long long DispenserBlockEntity::TYPE_ID = 0x0000000003C00200ULL;

DispenserBlockEntity::DispenserBlockEntity()
    : mItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(9, ItemInstance::EMPTY))
{
}

DispenserBlockEntity::~DispenserBlockEntity()
{
}

BlockEntity* DispenserBlockEntity::create()
{
    return new DispenserBlockEntity();
}

std::wstring DispenserBlockEntity::getName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring(L"container.dispenser");
}

std::wstring DispenserBlockEntity::getCustomName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring();
}

unsigned int DispenserBlockEntity::getContainerSize()
{
    return 9;
}

bool DispenserBlockEntity::isEmpty()
{
    for (unsigned int slot = 0; slot < mItems.size(); ++slot)
    {
        if (!mItems[slot]->isEmpty())
        {
            return false;
        }
    }
    return true;
}

int DispenserBlockEntity::getMaxStackSize()
{
    return 64;
}

void DispenserBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

AbstractContainerMenu* DispenserBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory,
                                                          boost::shared_ptr<Player> player)
{
    unpackLootTable(player);
    boost::shared_ptr<DispenserBlockEntity> self =
        boost::static_pointer_cast<DispenserBlockEntity>(shared_from_this());
    return new DispenserMenu(boost::static_pointer_cast<Container>(inventory),
                             boost::static_pointer_cast<Container>(self));
}

int DispenserBlockEntity::getTypeId(bool large)
{
    return 20;
}

int DispenserBlockEntity::getRandomSlot()
{
    boost::shared_ptr<Player> player;
    unpackLootTable(player);

    int count = 1;
    int selected = -1;
    for (unsigned int slot = 0; slot < mItems.size(); ++slot)
    {
        if (!mItems[slot]->isEmpty())
        {
            if (gDispenserRandom.nextInt(count) == 0)
            {
                selected = static_cast<int>(slot);
            }
            ++count;
        }
    }
    return selected;
}

int DispenserBlockEntity::addItem(not_null_ptr<ItemInstance> item)
{
    for (unsigned int slot = 0; slot < mItems.size(); ++slot)
    {
        if (mItems[slot]->isEmpty())
        {
            setItem(slot, item);
            return static_cast<int>(slot);
        }
    }
    return -1;
}

void DispenserBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     (DataWalker*)new ItemListDataWalker(DispenserBlockEntity::TYPE_ID, L"Items"));
}

void DispenserBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
    if (!tryLoadLootTable(tag))
    {
        ContainerHelper::loadAllItems(tag, mItems);
    }
    mCustomName->loadFromTag(tag);
}

CompoundTag* DispenserBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    if (!trySaveLootTable(tag))
    {
        ContainerHelper::saveAllItems(tag, mItems);
    }
    mCustomName->saveToTag(tag);
    return tag;
}

NonNullList<not_null_ptr<ItemInstance> > DispenserBlockEntity::getItems()
{
    return mItems;
}

void DispenserBlockEntity::setItems(NonNullList<not_null_ptr<ItemInstance> > items)
{
    mItems = items;
}

boost::shared_ptr<BlockEntity> DispenserBlockEntity::clone()
{
    boost::shared_ptr<DispenserBlockEntity> result(new DispenserBlockEntity());
    BlockEntity::clone(result);
    result->mCustomName->set(mCustomName->raw(), mCustomName->getUUID(), mCustomName->isChatCensored(), mCustomName->isUUIDLocal());
    for (unsigned int slot = 0; slot < mItems.size(); ++slot)
    {
        if (!mItems[slot]->isEmpty())
        {
            result->mItems[slot] = mItems[slot]->copy();
        }
    }
    return result;
}

unsigned long long DispenserBlockEntity::GetType()
{
    return DispenserBlockEntity::TYPE_ID;
}
