#include "game/ui/ChestMenu.hpp"

#include "game/core/GenericStats.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Slot.hpp"
#include "game/core/StatsCounter.hpp"
#include "game/item/Container.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

ChestMenu::ChestMenu(boost::shared_ptr<Container> container,
                     boost::shared_ptr<Container> playerInventory,
                     boost::shared_ptr<Player> player)
    : m_container(container),
      m_numRows((int)container->getContainerSize() / 9)
{
    container->startOpen(player);

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
            addSlot(new Slot(playerInventory, column + row * 9 + 9,
                             8 + column * 18, 18 + m_numRows * 18 + row * 18 + 13));
        }
    }
    for (column = 0; column < 9; ++column) {
        addSlot(new Slot(playerInventory, column,
                         8 + column * 18, 18 + m_numRows * 18 + 71));
    }
}

bool ChestMenu::stillValid(boost::shared_ptr<Player> player)
{
    return m_container->stillValid(player);
}

not_null_ptr<ItemInstance> ChestMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    Slot* slot = m_slots.at(index);
    if (slot == NULL || !slot->hasItem())
        return ItemInstance::EMPTY;

    not_null_ptr<ItemInstance> item = slot->getItem();
    not_null_ptr<ItemInstance> result = item->copy();
    int containerEnd = m_numRows * 9;

    if (index < containerEnd)
    {
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
        {
            bool replacedExisting = false;
            if (player->AutoEquip(result, item, replacedExisting))
            {
                slot->set(ItemInstance::EMPTY);
                return ItemInstance::EMPTY;
            }
        }

        if (!moveItemStackTo(item, containerEnd, (int)m_slots.size(), true, true, false))
        {
            if (!IsSmallInventory())
            {
                if (!item->getItem()->PreferInventory())
                {
                    if (!moveItemStackTo(item, containerEnd + 27, (int)m_slots.size(), false, false, true))
                        moveItemStackTo(item, containerEnd, containerEnd + 27, true, false, true);
                }
                else
                {
                    if (!moveItemStackTo(item, containerEnd, containerEnd + 27, true, false, true))
                        moveItemStackTo(item, containerEnd + 27, (int)m_slots.size(), false, false, true);
                }
            }
            else
            {
                moveItemStackTo(item, containerEnd + 27, (int)m_slots.size(), false, true, true);
            }
        }

        if (item->getCount() == 0)
            slot->set(ItemInstance::EMPTY);
        else
            slot->setChanged();
        if (item->getCount() == result->getCount())
            return ItemInstance::EMPTY;
        return result;
    }

    if (!moveItemStackTo(item, 0, containerEnd, false, true, true))
        return ItemInstance::EMPTY;

    if (item->getCount() == 0)
        slot->set(ItemInstance::EMPTY);
    else
        slot->setChanged();
    if (item->getCount() == result->getCount())
        return ItemInstance::EMPTY;
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
    not_null_ptr<ItemInstance> carried = player->mInventory->getCarried();
    bool carriedIsCobblestone = Block::byItem(carried->getItem()) == Blocks::cobblestone;

    bool clickedCobblestone = false;
    if (type == ClickType::eClickType_QuickMove && slot != -999 && m_slots[slot] != NULL)
    {
        not_null_ptr<ItemInstance> slotItem = m_slots[slot]->getItem();
        clickedCobblestone = Block::byItem(slotItem->getItem()) == Blocks::cobblestone;
    }

    not_null_ptr<ItemInstance> result = AbstractContainerMenu::clicked(slot, button, type, player);

    boost::shared_ptr<LocalPlayer> localPlayer = boost::dynamic_pointer_cast<LocalPlayer>(player);
    if (localPlayer && (carriedIsCobblestone || clickedCobblestone))
    {
        int cobblestoneCount = 0;
        unsigned int i;
        for (i = 0; i < m_container->getContainerSize(); ++i)
        {
            not_null_ptr<ItemInstance> containerItem = m_container->getItem(i);
            if (Block::byItem(containerItem->getItem()) == Blocks::cobblestone)
                cobblestoneCount += containerItem->getCount();
        }

        StatsCounter* stats = Minecraft::GetInstance()->mStats[localPlayer->mUserIndex];
        int stoneMined = stats->getTotalValue(gStats->get_blocksMined(Block::getId(Blocks::stone)));
        int cobblestoneMined = stats->getTotalValue(gStats->get_blocksMined(Block::getId(Blocks::cobblestone)));

        if (cobblestoneCount > 0x6bf && stoneMined + cobblestoneMined > 0x6bf)
        {
            localPlayer->awardStat(GenericStats::chestfulOfCobblestone(),
                                   GenericStats::param_chestfulOfCobblestone(cobblestoneCount));
        }
    }

    return result;
}
