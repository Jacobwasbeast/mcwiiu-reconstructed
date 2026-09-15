#include "game/ui/HopperMenu.hpp"

#include "game/core/Slot.hpp"
#include "game/item/ItemInstance.hpp"

HopperMenu::HopperMenu(boost::shared_ptr<Container> playerContainer,
                       boost::shared_ptr<Container> container,
                       boost::shared_ptr<Player> player)
{
    m_container = container;
    m_container->startOpen(player);

    unsigned int i;
    for (i = 0; i < m_container->getContainerSize(); ++i) {
        addSlot(new Slot(m_container, (int)i, 44 + (int)i * 18, 20));
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

HopperMenu::~HopperMenu()
{
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
        unsigned int containerSize = m_container->getContainerSize();
        bool moved;
        if ((unsigned int)index < containerSize) {
            moved = moveItemStackTo(item, (int)containerSize + 27,
                                    (int)m_slots.size(), false, true, true) != 0;
            if (!moved) {
                moved = moveItemStackTo(item, (int)containerSize,
                                        (int)m_slots.size(), true, true, true) != 0;
            }
        } else {
            moved = moveItemStackTo(item, 0, (int)containerSize,
                                    false, true, true) != 0;
        }
        if (!moved) {
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
