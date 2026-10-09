#include "game/ui/ShulkerBoxMenu.hpp"

#include "game/core/ShulkerBoxSlot.hpp"
#include "game/item/Container.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/core/Slot.hpp"

ShulkerBoxMenu::ShulkerBoxMenu(boost::shared_ptr<Container> inventory,
                               boost::shared_ptr<Container> container,
                               boost::shared_ptr<Player> player)
    : AbstractContainerMenu()
{
    int row;
    int column;

    mContainer = container;
    container->startOpen(player);

    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new ShulkerBoxSlot(container, column + row * 9,
                                       8 + column * 18, 18 + row * 18));
        }
    }

    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(inventory, column + row * 9 + 9,
                             8 + column * 18, 84 + row * 18));
        }
    }

    for (column = 0; column < 9; ++column)
    {
        addSlot(new Slot(inventory, column, 8 + column * 18, 142));
    }
}

bool ShulkerBoxMenu::stillValid(boost::shared_ptr<Player> player)
{
    return mContainer->stillValid(player);
}

not_null_ptr<ItemInstance> ShulkerBoxMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    Slot* slot = m_slots[index];

    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> item = slot->getItem();
        result = item->copy();

        if (index < (int)mContainer->getContainerSize())
        {
            if (!moveItemStackTo(item, (int)mContainer->getContainerSize(),
                                (int)m_slots.size(), true, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (!moveItemStackTo(item, 0, (int)mContainer->getContainerSize(),
                                 false, true, true))
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
    }

    return result;
}

void ShulkerBoxMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    mContainer->stopOpen(player);
}
