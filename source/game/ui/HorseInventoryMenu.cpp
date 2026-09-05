#include "game/ui/HorseInventoryMenu.hpp"

#include "game/core/Slot.hpp"
#include "game/entity/mob/AbstractChestedHorse.hpp"
#include "game/entity/mob/HorseArmorSlot.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/Player.hpp"

HorseInventoryMenu::HorseInventoryMenu(boost::shared_ptr<Container> horseContainer,
                                       boost::shared_ptr<Container> playerInventory,
                                       boost::shared_ptr<AbstractHorse> horse,
                                       boost::shared_ptr<Player> player)
    : m_container(horseContainer), horse(horse)
{
    horseContainer->startOpen(player);

    addSlot(new Slot(horseContainer, 0, 8, 18));
    addSlot(new HorseArmorSlot(this, horseContainer, 1, 8, 36));

    boost::shared_ptr<AbstractChestedHorse> chestedHorse =
        boost::static_pointer_cast<AbstractChestedHorse>(horse);
    if (chestedHorse->hasChest())
    {
        int columns = chestedHorse->getInventoryColumns();
        int row;
        int column;
        for (row = 0; row < 3; ++row)
        {
            for (column = 0; column < columns; ++column)
            {
                addSlot(new Slot(horseContainer, 2 + column + row * columns,
                                 80 + column * 18, 18 + row * 18));
            }
        }
    }

    int row;
    int column;
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(playerInventory, 9 + column + row * 9,
                             8 + column * 18, 84 + row * 18));
        }
    }
    for (column = 0; column < 9; ++column)
        addSlot(new Slot(playerInventory, column, 8 + column * 18, 142));
}

HorseInventoryMenu::~HorseInventoryMenu()
{
}

bool HorseInventoryMenu::stillValid(boost::shared_ptr<Player> player)
{
    if (!m_container->stillValid(player))
        return false;
    if (!horse->isAlive())
        return false;

    return horse->distanceTo(player) < 5.0;
}

not_null_ptr<ItemInstance> HorseInventoryMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    if (index < 0 || index >= (int)m_slots.size())
        return ItemInstance::EMPTY;

    Slot* slot = m_slots.at(index);
    if (slot == NULL || !slot->hasItem())
        return ItemInstance::EMPTY;

    not_null_ptr<ItemInstance> oldItem = slot->getItem();
    not_null_ptr<ItemInstance> item = oldItem->copy();
    int horseSize = (int)m_container->getContainerSize();
    bool moved = false;

    if (index < horseSize)
    {
        moved = moveItemStackTo(item, horseSize + 27, (int)m_slots.size(),
                                false, true, true) != 0;
        if (!moved)
            moved = moveItemStackTo(item, horseSize, horseSize + 27,
                                    false, true, true) != 0;
    }
    else
    {
        Slot* armorSlot = getSlot(1);
        if (armorSlot->mayPlace(item) && !armorSlot->hasItem())
            moved = moveItemStackTo(item, 1, 2, false, true, true) != 0;
        Slot* saddleSlot = getSlot(0);
        if (!moved && saddleSlot->mayPlace(item) && !saddleSlot->hasItem())
            moved = moveItemStackTo(item, 0, 1, false, true, true) != 0;

        if (!moved)
        {
            int playerInventoryEnd = horseSize + 27;
            int playerSlotsEnd = horseSize + 36;
            if (index >= playerInventoryEnd && index < playerSlotsEnd)
                moved = moveItemStackTo(item, horseSize, playerInventoryEnd,
                                        false, true, true) != 0;
            else if (index >= horseSize && index < playerInventoryEnd)
                moved = moveItemStackTo(item, playerInventoryEnd, playerSlotsEnd,
                                        false, true, true) != 0;
            else
                moved = false;
        }
    }

    if (!moved)
        return ItemInstance::EMPTY;
    if (item->isEmpty())
        slot->set(ItemInstance::EMPTY);
    else
        slot->setChanged();
    if (item->getCount() == oldItem->getCount())
        return ItemInstance::EMPTY;
    slot->onTake(player, item);
    return oldItem;
}

void HorseInventoryMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    m_container->stopOpen(player);
}
