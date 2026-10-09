#include "game/ui/HopperMenu.hpp"

#include "game/core/Slot.hpp"
#include "game/item/ItemInstance.hpp"

HopperMenu::HopperMenu(boost::shared_ptr<Container> playerContainer,
                       boost::shared_ptr<Container> container,
                       boost::shared_ptr<Player> player)
{
    m_container = container;
    container->startOpen(player);

    unsigned int i;
    for (i = 0; i < container->getContainerSize(); ++i) {
        addSlot(new Slot(container, (int)i, 44 + (int)i * 18, 20));
    }

    int row;
    int column;
    for (row = 0; row < 3; ++row) {
        for (column = 0; column < 9; ++column) {
            addSlot(new Slot(playerContainer, 9 + row * 9 + column,
                             8 + column * 18, 51 + row * 18));
        }
    }
    for (column = 0; column < 9; ++column) {
        addSlot(new Slot(playerContainer, column, 8 + column * 18, 109));
    }
}

bool HopperMenu::stillValid(boost::shared_ptr<Player> player)
{
    return m_container->stillValid(player);
}

not_null_ptr<ItemInstance> HopperMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    if (slot != NULL && slot->hasItem()) {
        not_null_ptr<ItemInstance> item = slot->getItem();
        result = item->copy();
        if ((unsigned int)index < m_container->getContainerSize()) {
            if (!moveItemStackTo(item, (int)m_container->getContainerSize() + 27,
                                (int)m_slots.size(), false, true, true)) {
                if (!moveItemStackTo(item, (int)m_container->getContainerSize(),
                                    (int)m_slots.size(), true, true, true)) {
                    return ItemInstance::EMPTY;
                }
            }
        } else if (!moveItemStackTo(item, 0, (int)m_container->getContainerSize(),
                                   false, true, true)) {
            return ItemInstance::EMPTY;
        }
        if (item->getCount() == 0) {
            slot->set(ItemInstance::EMPTY);
        } else {
            slot->setChanged();
        }
    }
    return result;
}

void HopperMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    m_container->stopOpen(player);
}
