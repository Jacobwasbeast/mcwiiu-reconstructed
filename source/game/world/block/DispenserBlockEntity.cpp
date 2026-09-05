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
#include "game/util/Math.hpp"
#include "game/world/block/BlockEntity.hpp"

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

    unsigned int count = 0;
    int selected = -1;
    for (unsigned int slot = 0; slot < mItems.size(); ++slot)
    {
        if (!mItems[slot]->isEmpty())
        {
            ++count;
            if (Math::randomInt(static_cast<int>(count)) == 0)
            {
                selected = static_cast<int>(slot);
            }
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
                     (DataWalker*)new ItemListDataWalker(0x3c00200ULL, L"Items"));
}

void DispenserBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
    if (!tryLoadLootTable(tag))
    {
        ContainerHelper::loadAllItems(tag, mItems);
    }
    if (mCustomName)
    {
        mCustomName->loadFromTag(tag);
    }
}

void DispenserBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    if (!trySaveLootTable(tag))
    {
        ContainerHelper::saveAllItems(tag, mItems);
    }
    if (mCustomName)
    {
        mCustomName->saveToTag(tag);
    }
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
    result->mItems = mItems;
    if (mCustomName)
    {
        result->mCustomName->set(mCustomName->raw(), std::wstring(), false, false);
    }
    return result;
}

unsigned long long DispenserBlockEntity::GetType()
{
    return 0x003c002000000000ULL;
}
