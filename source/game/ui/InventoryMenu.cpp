#include "game/ui/InventoryMenu.hpp"

#include "game/core/ArmorSlot.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/ResultSlot.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ResultContainer.hpp"

InventoryMenu::InventoryMenu(boost::shared_ptr<Inventory> inventory, bool active, Player* player)
    : mPlayer(player), mActive(active)
{
    _init(inventory, active);
}

InventoryMenu::~InventoryMenu()
{
}

void InventoryMenu::staticCtor()
{
}

void InventoryMenu::_init(boost::shared_ptr<Inventory> inventory, bool active)
{
    mActive = active;
    mCraftSlots.reset(new CraftingContainer(this, 2, 2));
    mResultSlots.reset(new ResultContainer());

    addSlot(new ResultSlot(mPlayer, mCraftSlots, mResultSlots, 0, 154, 28));

    int row;
    int column;
    for (row = 0; row < 2; ++row)
    {
        for (column = 0; column < 2; ++column)
        {
            addSlot(new Slot(mCraftSlots, column + row * 2, 98 + column * 18, 18 + row * 18));
        }
    }

    addSlot(new ArmorSlot(EquipmentSlot::HEAD, inventory, 39, 62, 8));
    addSlot(new ArmorSlot(EquipmentSlot::CHEST, inventory, 38, 62, 26));
    addSlot(new ArmorSlot(EquipmentSlot::LEGS, inventory, 37, 62, 44));
    addSlot(new ArmorSlot(EquipmentSlot::FEET, inventory, 36, 62, 62));

    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(inventory, column + row * 9 + 9, 8 + column * 18, 84 + row * 18));
        }
    }

    for (column = 0; column < 9; ++column)
    {
        addSlot(new Slot(inventory, column, 8 + column * 18, 142));
    }

    addSlot(new OffhandSlot(inventory, 40, 77, 62));
    slotsChanged();
}

void InventoryMenu::slotsChanged()
{
    if (mCraftSlots)
    {
        mCraftSlots->setChanged();
    }
}

void InventoryMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    if (mCraftSlots)
    {
        mCraftSlots->clearContent();
    }
    if (mResultSlots)
    {
        mResultSlots->clearContent();
    }
}

bool InventoryMenu::stillValid(boost::shared_ptr<Player> player)
{
    return true;
}

not_null_ptr<ItemInstance> InventoryMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    Slot* slot = getSlot(index);
    if (slot == NULL || !slot->hasItem())
    {
        return ItemInstance::EMPTY;
    }

    not_null_ptr<ItemInstance> item = slot->getItem();
    not_null_ptr<ItemInstance> result = item->copy();
    if (index == 0)
    {
        if (!moveItemStackTo(item, 9, 45, true, true, false))
        {
            return ItemInstance::EMPTY;
        }
    }
    else if (index < 9)
    {
        if (!moveItemStackTo(item, 9, 45, false, true, false))
        {
            return ItemInstance::EMPTY;
        }
    }
    else if (index < 45)
    {
        if (!moveItemStackTo(item, 0, 9, false, true, false) &&
            !moveItemStackTo(item, 45, 46, false, true, false))
        {
            return ItemInstance::EMPTY;
        }
    }
    else if (!moveItemStackTo(item, 9, 45, false, true, false))
    {
        return ItemInstance::EMPTY;
    }

    if (item->isEmpty())
    {
        slot->set(ItemInstance::EMPTY);
    }
    else
    {
        slot->setChanged();
    }
    return result;
}

bool InventoryMenu::mayCombine(Slot* slot, not_null_ptr<ItemInstance> item)
{
    return slot != NULL && slot->mayCombine(item);
}

bool InventoryMenu::canTakeItemForPickAll(not_null_ptr<ItemInstance> item, Slot* slot)
{
    return slot != NULL && slot != getSlot(0) && AbstractContainerMenu::canTakeItemForPickAll(item, slot);
}

not_null_ptr<ItemInstance> InventoryMenu::clicked(int slot, int button, ClickType::eClickType type,
                                                  boost::shared_ptr<Player> player)
{
    return AbstractContainerMenu::clicked(slot, button, type, player);
}

InventoryMenu::OffhandSlot::OffhandSlot(boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y)
{
}

InventoryMenu::OffhandSlot::~OffhandSlot()
{
}

bool InventoryMenu::OffhandSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return item != ItemInstance::EMPTY && !item->isEmpty();
}
