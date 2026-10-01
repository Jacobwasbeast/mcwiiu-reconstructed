#include "game/core/MerchantResultSlot.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/item/MerchantRecipe.hpp"
#include "game/world/generation/Player.hpp"

MerchantResultSlot::MerchantResultSlot(Player* player, boost::shared_ptr<Merchant> merchant,
                                       boost::shared_ptr<MerchantContainer> merchantContainer,
                                       int slot, int x, int y)
    : Slot(merchantContainer, slot, x, y),
      mMerchantContainer(merchantContainer),
      mPlayer(player),
      mRemoveCount(0),
      mMerchant(merchant)
{
}

bool MerchantResultSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return false;
}

not_null_ptr<ItemInstance> MerchantResultSlot::remove(int count)
{
    if (hasItem())
    {
        not_null_ptr<ItemInstance> item = getItem();
        int removed = count < item->getCount() ? count : item->getCount();
        mRemoveCount += removed;
    }
    return Slot::remove(count);
}

void MerchantResultSlot::onQuickCraft(not_null_ptr<ItemInstance> item, int count)
{
    mRemoveCount += count;
    checkTakeAchievements(item);
}

void MerchantResultSlot::checkTakeAchievements(not_null_ptr<ItemInstance> item)
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(mPlayer->shared_from_this());
    item->onCraftedBy(mPlayer->mLevel, player, mRemoveCount);
    mRemoveCount = 0;
}

bool MerchantResultSlot::removePaymentItemsIfMatching(MerchantRecipe* recipe,
                                                       not_null_ptr<ItemInstance> first,
                                                       not_null_ptr<ItemInstance> second)
{
    not_null_ptr<ItemInstance> buyA = recipe->getBuyAItem();
    not_null_ptr<ItemInstance> buyB = recipe->getBuyBItem();

    if (first->getItem() != buyA->getItem() || first->getCount() < buyA->getCount())
    {
        return false;
    }

    if (!buyB->isEmpty())
    {
        if (second->isEmpty() || second->getItem() != buyB->getItem() ||
            second->getCount() < buyB->getCount())
        {
            return false;
        }
        first->shrink(buyA->getCount());
        second->shrink(buyB->getCount());
        return true;
    }

    if (!second->isEmpty())
    {
        return false;
    }
    first->shrink(buyA->getCount());
    return true;
}

not_null_ptr<ItemInstance> MerchantResultSlot::onTake(boost::shared_ptr<Player> player,
                                                      not_null_ptr<ItemInstance> item)
{
    checkTakeAchievements(item);

    MerchantRecipe* recipe = mMerchantContainer->getActiveRecipe();
    if (recipe == NULL)
    {
        return item;
    }

    not_null_ptr<ItemInstance> first = mMerchantContainer->getItem(0);
    not_null_ptr<ItemInstance> second = mMerchantContainer->getItem(1);
    bool traded = removePaymentItemsIfMatching(recipe, first, second);
    if (!traded)
    {
        traded = removePaymentItemsIfMatching(recipe, second, first);
    }
    if (traded)
    {
        mMerchant->notifyTrade(recipe);
        mMerchantContainer->setItem(0, first);
        mMerchantContainer->setItem(1, second);
    }

    return item;
}

bool MerchantResultSlot::mayCombine(not_null_ptr<ItemInstance> item)
{
    return false;
}
