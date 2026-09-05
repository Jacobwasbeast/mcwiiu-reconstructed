#include "game/ui/CraftingMenu.hpp"

#include "game/core/ResultSlot.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Recipes.hpp"
#include "game/item/ResultContainer.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/entity/mob/Entity.hpp"

CraftingMenu::CraftingMenu(boost::shared_ptr<Inventory> inventory, Level* level, const BlockPos& pos)
    : AbstractContainerMenu(),
      m_craftSlots(new CraftingContainer(this, 3, 3)),
      m_resultSlots(new ResultContainer()),
      m_level(level),
      m_playerPos(pos)
{
    addSlot(new ResultSlot(NULL, m_craftSlots, m_resultSlots, 0, 124, 35));

    int row;
    int column;
    for (row = 0; row < 3; ++row)
        for (column = 0; column < 3; ++column)
            addSlot(new Slot(m_craftSlots, row * 3 + column, 30 + column * 18, 17 + row * 18));

    for (row = 0; row < 3; ++row)
        for (column = 0; column < 9; ++column)
            addSlot(new Slot(inventory, 9 + row * 9 + column, 8 + column * 18, 84 + row * 18));

    for (column = 0; column < 9; ++column)
        addSlot(new Slot(inventory, column, 8 + column * 18, 142));

    slotsChanged();
}

CraftingMenu::~CraftingMenu()
{
}

void CraftingMenu::slotsChanged()
{
    not_null_ptr<ItemInstance> result =
        Recipes::getInstance()->getItemFor(m_craftSlots, m_level, NULL, Recipes::eRecipeCategory_Crafting);
    m_resultSlots->setItem(0, result);
}

void CraftingMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    if (!m_level->mIsClientSide)
        clearContainer(player, m_level, m_craftSlots);
}

bool CraftingMenu::stillValid(boost::shared_ptr<Player> player)
{
    if (m_level->getBlockState(m_playerPos)->getBlock() != Blocks::crafting_table)
        return false;
    Entity* entity = (Entity*)player.get();
    return entity->distanceAsIfHitscanned(m_playerPos, *entity->mBoundingBox) <= 30.25;
}

not_null_ptr<ItemInstance> CraftingMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    if (index < 0 || index >= (int)m_slots.size())
        return ItemInstance::EMPTY;

    Slot* slot = m_slots[index];
    if (slot == NULL || !slot->hasItem() || !slot->mayPickup(player))
        return ItemInstance::EMPTY;

    not_null_ptr<ItemInstance> oldItem = slot->getItem();
    not_null_ptr<ItemInstance> item = oldItem->copy();
    bool moved;

    if (index == 0)
    {
        moved = moveItemStackTo(item, 10, 46, true, true, true) != 0;
        if (!moved)
            return ItemInstance::EMPTY;
        slot->onQuickCraft(oldItem, item);
    }
    else if (index >= 10 && index < 37)
    {
        moved = moveItemStackTo(item, 37, 46, false, true, true) != 0;
        if (!moved)
            moved = moveItemStackTo(item, 0, 10, false, true, true) != 0;
        if (!moved)
            return ItemInstance::EMPTY;
    }
    else if (index >= 37 && index < 46)
    {
        moved = moveItemStackTo(item, 0, 10, false, true, true) != 0;
        if (!moved)
            return ItemInstance::EMPTY;
    }
    else
    {
        moved = moveItemStackTo(item, 10, 46, false, true, true) != 0;
        if (!moved)
            return ItemInstance::EMPTY;
    }

    if (item->isEmpty())
        slot->set(ItemInstance::EMPTY);
    else
        slot->setChanged();

    if (item->getCount() == oldItem->getCount())
        return ItemInstance::EMPTY;
    slot->onTake(player, item);
    return oldItem;
}

bool CraftingMenu::canTakeItemForPickAll(not_null_ptr<ItemInstance> item, Slot* slot)
{
    return slot != m_slots[0] && AbstractContainerMenu::canTakeItemForPickAll(item, slot);
}
