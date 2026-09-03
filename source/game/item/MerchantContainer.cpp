#include "game/item/MerchantContainer.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/MerchantRecipe.hpp"
#include "game/item/MerchantRecipeList.hpp"
#include "game/core/Merchant.hpp"

MerchantContainer::MerchantContainer(boost::shared_ptr<Player> player, boost::shared_ptr<Merchant> merchant)
    : Container(), mItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(3, ItemInstance::EMPTY)),
      mMerchant(merchant), mTradingPlayer(player), mOffers(0), mSelectionHint(0), mActiveRecipe(0)
{
    if (mMerchant)
    {
        mOffers = mMerchant->getOffers(player);
        mMerchant->setTradingPlayer(player);
    }
}

MerchantContainer::~MerchantContainer()
{
}

std::wstring MerchantContainer::getName()
{
    return mMerchant ? mMerchant->getDisplayName() : std::wstring();
}

std::wstring MerchantContainer::getCustomName()
{
    return std::wstring();
}

bool MerchantContainer::hasCustomName()
{
    return false;
}

std::wstring MerchantContainer::getDisplayName()
{
    return getName();
}

void MerchantContainer::updateSellItem()
{
    mActiveRecipe = 0;
    mItems[2] = ItemInstance::EMPTY;
    if (mOffers == 0)
    {
        return;
    }
    MerchantRecipe* recipe = mOffers->getMatchingRecipeFor(mItems[0], mItems[1], ItemInstance::EMPTY);
    if (recipe != 0)
    {
        mActiveRecipe = recipe;
        mItems[2] = recipe->getSellItem();
    }
}

void MerchantContainer::setSelectionHint(int hint)
{
    mSelectionHint = hint;
    updateSellItem();
}

unsigned int MerchantContainer::getContainerSize()
{
    return mItems.size();
}

bool MerchantContainer::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        if (!mItems[i]->isEmpty())
        {
            return false;
        }
    }
    return true;
}

not_null_ptr<ItemInstance> MerchantContainer::getItem(unsigned int slot)
{
    return mItems[slot];
}

bool MerchantContainer::isPaymentSlot(int slot)
{
    return slot == 0 || slot == 1;
}

not_null_ptr<ItemInstance> MerchantContainer::removeItem(unsigned int slot, int count)
{
    not_null_ptr<ItemInstance> removed = ContainerHelper::removeItem(mItems, slot, (unsigned int)count);
    if (isPaymentSlot((int)slot))
    {
        updateSellItem();
    }
    return removed;
}

not_null_ptr<ItemInstance> MerchantContainer::removeItemNoUpdate(int slot)
{
    return ContainerHelper::takeItem(mItems, (unsigned int)slot);
}

void MerchantContainer::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    mItems[slot] = item;
    if (!item->isEmpty() && item->getCount() > getMaxStackSize())
    {
        item->setCount(getMaxStackSize());
    }
    if (isPaymentSlot((int)slot))
    {
        updateSellItem();
    }
}

int MerchantContainer::getMaxStackSize()
{
    return 64;
}

bool MerchantContainer::stillValid(boost::shared_ptr<Player> player)
{
    return mMerchant && mMerchant->getTradingPlayer() == player;
}

void MerchantContainer::startOpen(boost::shared_ptr<Player>)
{
}

void MerchantContainer::stopOpen(boost::shared_ptr<Player>)
{
}

bool MerchantContainer::canPlaceItem(int, not_null_ptr<ItemInstance>)
{
    return true;
}

void MerchantContainer::setChanged()
{
    updateSellItem();
}

MerchantRecipe* MerchantContainer::getActiveRecipe()
{
    return mActiveRecipe;
}

int MerchantContainer::getContainerData(int)
{
    return 0;
}

void MerchantContainer::setContainerData(int, int)
{
}

int MerchantContainer::getContainerDataCount()
{
    return 0;
}

void MerchantContainer::clearContent()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        mItems[i] = ItemInstance::EMPTY;
    }
    mActiveRecipe = 0;
}
