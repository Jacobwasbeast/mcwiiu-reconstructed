#include "game/ui/TrapMenu.hpp"

#include "game/core/Slot.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"

TrapMenu::TrapMenu(boost::shared_ptr<Container> playerInventory, boost::shared_ptr<Container> container)
{
    int row;
    int column;

    m_container = container;

    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 3; ++column)
        {
            addSlot(new Slot(container, row * 3 + column, 62 + column * 18, 17 + row * 18));
        }
    }

    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(playerInventory, column + row * 9 + 9, 8 + column * 18, 84 + row * 18));
        }
    }

    for (column = 0; column < 9; ++column)
    {
        addSlot(new Slot(playerInventory, column, 8 + column * 18, 142));
    }
}

bool TrapMenu::stillValid(boost::shared_ptr<Player> player)
{
    return m_container->stillValid(player);
}

not_null_ptr<ItemInstance> TrapMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> item = slot->getItem();
        result = item->copy();
        if (index < 9)
        {
            if (!moveItemStackTo(item, 9, 45, true, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (!moveItemStackTo(item, 0, 9, false, true, true))
        {
            return ItemInstance::EMPTY;
        }

        if (item->getCount() == 0)
        {
            slot->set(ItemInstance::EMPTY);
        }
        else
        {
            slot->setChanged();
        }

        if (item->getCount() == result->getCount())
        {
            return ItemInstance::EMPTY;
        }

        slot->onTake(player, item);
    }

    return result;
}
