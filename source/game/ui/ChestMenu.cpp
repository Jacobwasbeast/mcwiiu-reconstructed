#include "game/ui/ChestMenu.hpp"

#include "game/core/Slot.hpp"
#include "game/item/ItemInstance.hpp"

ChestMenu::ChestMenu(boost::shared_ptr<Container> container,
                     boost::shared_ptr<Container> playerInventory,
                     boost::shared_ptr<Player> player)
    : m_container(container), m_playerInventory(playerInventory),
      m_numRows((int)container->getContainerSize() / 9)
{
    int row;
    int column;

    for (row = 0; row < m_numRows; ++row) {
        for (column = 0; column < 9; ++column) {
            addSlot(new Slot(m_container, column + row * 9,
                             8 + column * 18, 18 + row * 18));
        }
    }
    for (row = 0; row < 3; ++row) {
        for (column = 0; column < 9; ++column) {
            addSlot(new Slot(m_playerInventory, column + row * 9 + 9,
                             8 + column * 18, 18 + m_numRows * 18 + row * 18 + 13));
        }
    }
    for (column = 0; column < 9; ++column) {
        addSlot(new Slot(m_playerInventory, column,
                         8 + column * 18, 18 + m_numRows * 18 + 71));
    }
}

ChestMenu::~ChestMenu()
{
}

bool ChestMenu::stillValid(boost::shared_ptr<Player> player)
{
    return m_container->stillValid(player);
}

not_null_ptr<ItemInstance> ChestMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    Slot* slot = getSlot(index);
    if (slot == NULL || !slot->hasItem()) return result;

    not_null_ptr<ItemInstance> item = slot->getItem();
    result = item->copy();
    int containerEnd = m_numRows * 9;
    bool moved;
    if (index < containerEnd) {
        moved = moveItemStackTo(item, containerEnd, (int)m_slots.size(), true, true, false) != 0;
    } else {
        moved = moveItemStackTo(item, 0, containerEnd, false, false, true) != 0;
    }
    if (!moved) return ItemInstance::EMPTY;
    if (item->isEmpty()) slot->set(ItemInstance::EMPTY);
    else slot->setChanged();
    if (item->getCount() == result->getCount()) return ItemInstance::EMPTY;
    slot->onTake(player, item);
    return result;
}

void ChestMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    m_container->stopOpen(player);
}

boost::shared_ptr<Container> ChestMenu::getContainer()
{
    return m_container;
}

not_null_ptr<ItemInstance> ChestMenu::clicked(int slot, int button,
                                              ClickType::eClickType type,
                                              boost::shared_ptr<Player> player)
{
    return AbstractContainerMenu::clicked(slot, button, type, player);
}
