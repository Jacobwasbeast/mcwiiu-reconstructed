#include "game/ui/AnvilMenu.hpp"

#include "game/core/RepairResultSlot.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/RepairContainer.hpp"
#include "game/item/ResultContainer.hpp"
#include "game/network/net_minecraft_world_inventory__ContainerListener.hpp"
#include "game/world/Level.hpp"

AnvilMenu::AnvilMenu(boost::shared_ptr<Inventory> inventory, Level* level,
                     const BlockPos& position, boost::shared_ptr<Player> player)
    : AbstractContainerMenu(), mResultSlots(), mInputSlots(), mLevel(NULL),
      mPosition(), mRepairCost(0), mMaterialCost(0), mItemName(),
      mItemNameCensored(false), mPlayer(NULL), mPlayerOwner()
{
    _init(inventory, level, position, player);
}

AnvilMenu::AnvilMenu(boost::shared_ptr<Inventory> inventory, Level* level,
                     boost::shared_ptr<Player> player)
    : AbstractContainerMenu(), mResultSlots(), mInputSlots(), mLevel(NULL),
      mPosition(), mRepairCost(0), mMaterialCost(0), mItemName(),
      mItemNameCensored(false), mPlayer(NULL), mPlayerOwner()
{
    _init(inventory, level, BlockPos::ZERO, player);
}

AnvilMenu::~AnvilMenu()
{
}

void AnvilMenu::_init(boost::shared_ptr<Inventory> inventory, Level* level,
                      const BlockPos& position, boost::shared_ptr<Player> player)
{
    mResultSlots = boost::shared_ptr<ResultContainer>(new ResultContainer());
    mInputSlots = boost::shared_ptr<RepairContainer>(new RepairContainer(this, 0xc60bb915, true, 2));
    mLevel = level;
    mPosition = position;
    mRepairCost = 0;
    mMaterialCost = 0;
    mItemName.clear();
    mItemNameCensored = false;
    mPlayer = player.get();
    mPlayerOwner = player;

    boost::shared_ptr<Container> input = mInputSlots;
    boost::shared_ptr<Container> result = mResultSlots;
    boost::shared_ptr<Container> playerInventory = inventory;
    addSlot(new Slot(input, 0, 27, 47));
    addSlot(new Slot(input, 1, 76, 47));
    addSlot(new RepairResultSlot(this, mPosition, result, 2, 134, 47));

    int row;
    int column;
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(playerInventory, column + row * 9 + 9,
                             8 + column * 18, 84 + row * 18));
        }
    }
    for (column = 0; column < 9; ++column)
    {
        addSlot(new Slot(playerInventory, column, 8 + column * 18, 142));
    }
}

void AnvilMenu::createResult()
{
    not_null_ptr<ItemInstance> first = mInputSlots->getItem(0);
    not_null_ptr<ItemInstance> second = mInputSlots->getItem(1);
    mRepairCost = 0;
    mMaterialCost = 0;
    if (first->isEmpty())
    {
        mResultSlots->setItem(0, ItemInstance::EMPTY);
        return;
    }

    not_null_ptr<ItemInstance> result = first->copy();
    if (!second->isEmpty())
    {
        if (first->isDamageableItem() && first->getItem() == second->getItem())
        {
            int remainingFirst = first->getMaxDamage() - first->getDamageValue();
            int remainingSecond = second->getMaxDamage() - second->getDamageValue();
            int repaired = remainingFirst + remainingSecond + first->getMaxDamage() * 12 / 100;
            int damage = first->getMaxDamage() - repaired;
            if (damage < 0) damage = 0;
            if (damage < result->getAuxValue())
            {
                result->setAuxValue(damage);
                mMaterialCost = 1;
            }
        }
    }
    if (mItemName.empty())
    {
        if (first->hasCustomHoverName())
        {
            result->resetHoverName();
            ++mRepairCost;
        }
    }
    else if (mItemName != first->getHoverNameUnfiltered())
    {
        result->setHoverName(mItemName);
        result->setCensorshipInfo(mItemName, mItemNameCensored);
        ++mRepairCost;
    }
    if (mRepairCost > 39 && mRepairCost != 1)
    {
        mResultSlots->setItem(0, ItemInstance::EMPTY);
        return;
    }
    if (mRepairCost > 0 || mMaterialCost > 0)
    {
        mResultSlots->setItem(0, result);
    }
    else
    {
        mResultSlots->setItem(0, ItemInstance::EMPTY);
    }
}

void AnvilMenu::slotsChanged(boost::shared_ptr<Container> container)
{
    AbstractContainerMenu::slotsChanged();
    if (container.get() == mInputSlots.get()) createResult();
}

void AnvilMenu::addSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    AbstractContainerMenu::addSlotListener(listener);
    listener->setContainerData(this, 0, mRepairCost);
}

void AnvilMenu::setData(int id, int value)
{
    if (id == 0) mRepairCost = value;
}

void AnvilMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    if (mLevel != NULL && !mLevel->mIsClientSide)
    {
        boost::shared_ptr<Container> input = mInputSlots;
        clearContainer(player, mLevel, input);
    }
}

bool AnvilMenu::stillValid(boost::shared_ptr<Player>)
{
    return mLevel != NULL;
}

not_null_ptr<ItemInstance> AnvilMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    Slot* slot = getSlot(index);
    if (slot == NULL || !slot->hasItem()) return ItemInstance::EMPTY;
    not_null_ptr<ItemInstance> item = slot->getItem();
    not_null_ptr<ItemInstance> copy = item->copy();
    bool moved;
    if (index == 2)
        moved = moveItemStackTo(item, 3, (int)m_slots.size(), true, true, true) != 0;
    else if (index < 3)
        moved = moveItemStackTo(item, 3, (int)m_slots.size(), true, true, true) != 0;
    else
        moved = moveItemStackTo(item, 0, 3, false, false, true) != 0;
    if (!moved) return ItemInstance::EMPTY;
    if (item->isEmpty()) slot->set(ItemInstance::EMPTY);
    else slot->setChanged();
    slot->onQuickCraft(item, copy);
    return copy;
}

void AnvilMenu::setItemName(const std::wstring& name, bool censored)
{
    mItemName = name;
    mItemNameCensored = censored;
    not_null_ptr<ItemInstance> result = mResultSlots->getItem(0);
    if (!result->isEmpty())
    {
        if (mItemName.empty()) result->resetHoverName();
        else
        {
            result->setHoverName(mItemName);
            result->setCensorshipInfo(mItemName, censored);
        }
    }
    createResult();
}
