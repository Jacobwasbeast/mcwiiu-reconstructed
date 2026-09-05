#include "game/ui/DispenserMenu.hpp"

#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/MiniGameDef.hpp"

DispenserMenu::DispenserMenu(boost::shared_ptr<Container> playerInventory,
                             boost::shared_ptr<Container> dispenser)
    : m_dispenser(dispenser), m_playerInventory(playerInventory)
{
    int row;
    int column;

    for (row = 0; row < 3; ++row) {
        for (column = 0; column < 3; ++column) {
            addSlot(new Slot(m_dispenser, row * 3 + column, 62 + column * 18, 17 + row * 18));
        }
    }

    for (row = 0; row < 3; ++row) {
        for (column = 0; column < 9; ++column) {
            addSlot(new Slot(m_playerInventory, 9 + row * 9 + column, 8 + column * 18, 84 + row * 18));
        }
    }

    for (column = 0; column < 9; ++column) {
        addSlot(new Slot(m_playerInventory, column, 8 + column * 18, 142));
    }
}

DispenserMenu::~DispenserMenu()
{
}

bool DispenserMenu::stillValid(boost::shared_ptr<Player> player)
{
    return m_dispenser->stillValid(player);
}

not_null_ptr<ItemInstance> DispenserMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);

    if (slot != NULL && slot->hasItem()) {
        not_null_ptr<ItemInstance> item = slot->getItem();
        result = item->copy();
        bool smallInventory = Minecraft::InMiniGame(eMiniGameId_Any, true)
            && Minecraft::GetInstance()->GetMiniGame()->IsSmallInventoryEnabled();
        int start = 0;
        int end = 0;

        if (index < 9) {
            start = smallInventory ? 36 : 9;
            end = 45;
        } else if (index < 45) {
            start = 0;
            end = 9;
        } else {
            return ItemInstance::EMPTY;
        }

        if (moveItemStackTo(item, start, end, false, true, true) != 1) {
            return ItemInstance::EMPTY;
        }

        if (item->getCount() == 0) {
            slot->set(ItemInstance::EMPTY);
        } else {
            slot->setChanged();
        }

        if (item->getCount() == result->getCount()) {
            return ItemInstance::EMPTY;
        }

        slot->onTake(player, item);
    }

    return result;
}
