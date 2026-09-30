#include "game/ui/MerchantMenu.hpp"

#include "game/core/Merchant.hpp"
#include "game/core/MerchantResultSlot.hpp"
#include "game/core/Slot.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/MerchantContainer.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

MerchantMenu::MerchantMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Merchant> merchant, Level* level)
    : AbstractContainerMenu(), mMerchant(merchant), mTradeContainer(), mLevel(level)
{
    boost::shared_ptr<Player> player = merchant ? merchant->getTradingPlayer() : boost::shared_ptr<Player>();
    mTradeContainer.reset(new MerchantContainer(player, merchant));
    addSlot(new Slot(mTradeContainer, 0, 36, 53));
    addSlot(new Slot(mTradeContainer, 1, 62, 53));
    addSlot(new MerchantResultSlot(player.get(),
                                   merchant, mTradeContainer, 2, 120, 53));

    int row;
    int column;
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(inventory, column + row * 9 + 9, 8 + column * 18, 84 + row * 18));
        }
    }
    for (column = 0; column < 9; ++column)
    {
        addSlot(new Slot(inventory, column, 8 + column * 18, 142));
    }
}

boost::shared_ptr<MerchantContainer> MerchantMenu::getTradeContainer()
{
    return mTradeContainer;
}

void MerchantMenu::slotsChanged()
{
    mTradeContainer->updateSellItem();
    AbstractContainerMenu::slotsChanged();
}

void MerchantMenu::setSelectionHint(int hint)
{
    mTradeContainer->setSelectionHint(hint);
}

bool MerchantMenu::stillValid(boost::shared_ptr<Player> player)
{
    return mMerchant->getTradingPlayer() == player;
}

not_null_ptr<ItemInstance> MerchantMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> empty = ItemInstance::EMPTY;
    if (index < 0 || index >= (int)m_slots.size())
    {
        return empty;
    }

    Slot* slot = m_slots[index];
    if (slot == NULL || !slot->hasItem())
    {
        return empty;
    }
    not_null_ptr<ItemInstance> item = slot->getItem();
    not_null_ptr<ItemInstance> original = item->copy();
    bool moved = false;

    if (index == 2)
    {
        moved = moveItemStackTo(item, 3, 39, true, true, true) == 1;
        if (!moved)
        {
            return empty;
        }
        slot->onQuickCraft(item, original);
    }
    else if (index == 0 || index == 1)
    {
        moved = moveItemStackTo(item, 3, 39, false, true, true) == 1;
    }
    else if (index >= 3 && index < 30)
    {
        moved = moveItemStackTo(item, 30, 39, false, true, true) == 1;
    }
    else if (index >= 30 && index < 39)
    {
        moved = moveItemStackTo(item, 3, 30, false, true, true) == 1;
    }
    else
    {
        return empty;
    }

    if (!moved)
    {
        return empty;
    }
    if (item->isEmpty())
    {
        slot->set(ItemInstance::EMPTY);
    }
    else
    {
        slot->setChanged();
    }
    if (item->getCount() == original->getCount())
    {
        return empty;
    }
    slot->onTake(player, item);
    return original;
}

void MerchantMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    AbstractContainerMenu::removed(player);
    if (!mLevel->mIsClientSide)
    {
        not_null_ptr<ItemInstance> first = mTradeContainer->getItem(0);
        if (!first->isEmpty())
        {
            player->drop(first, false);
        }
        not_null_ptr<ItemInstance> second = mTradeContainer->getItem(1);
        if (!second->isEmpty())
        {
            player->drop(second, false);
        }
    }
}

boost::shared_ptr<Merchant> MerchantMenu::getMerchant()
{
    return mMerchant;
}
