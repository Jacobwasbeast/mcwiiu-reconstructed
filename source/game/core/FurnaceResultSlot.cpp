#include "game/core/FurnaceResultSlot.hpp"

#include "game/item/ItemInstance.hpp"

FurnaceResultSlot::FurnaceResultSlot(boost::shared_ptr<Player> player,
                                     boost::shared_ptr<Container> container,
                                     int index, int x, int y)
    : Slot(container, index, x, y),
      mPlayer(player),
      mRemoveCount(0)
{
}

FurnaceResultSlot::~FurnaceResultSlot()
{
}

bool FurnaceResultSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return false;
}

not_null_ptr<ItemInstance> FurnaceResultSlot::remove(int count)
{
    if (hasItem())
    {
        not_null_ptr<ItemInstance> item = getItem();
        int removed = item->getCount();
        if (count < removed)
        {
            removed = count;
        }
        mRemoveCount += removed;
    }
    return Slot::remove(count);
}

void FurnaceResultSlot::onTake(boost::shared_ptr<Player> player,
                               not_null_ptr<ItemInstance> item)
{
    set(item);
    Slot::onTake(player, item);
}

bool FurnaceResultSlot::mayCombine(not_null_ptr<ItemInstance> item)
{
    return false;
}

void FurnaceResultSlot::onQuickCraft(not_null_ptr<ItemInstance> item, int count)
{
    mRemoveCount += count;
    checkTakeAchievements(item);
}

void FurnaceResultSlot::checkTakeAchievements(not_null_ptr<ItemInstance> item)
{
    item->onCraftedBy(NULL, mPlayer, mRemoveCount);
    mRemoveCount = 0;
}
