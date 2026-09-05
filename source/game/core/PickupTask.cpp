#include "game/core/PickupTask.hpp"

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/world/block/Blocks.hpp"

PickupTask::PickupTask(Item** items, unsigned int itemCount, unsigned int requiredCount, int auxValue,
                       Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                       std::vector<TutorialConstraint*>* constraints, bool showMinimumTime,
                       bool allowFade, bool repeatMessage, MobEffect* effect)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, constraints, showMinimumTime,
                   allowFade, repeatMessage),
      items(new Item*[itemCount]), itemCount(itemCount), requiredCount(requiredCount),
      auxValue(auxValue), effect(effect)
{
    unsigned int i;
    for (i = 0; i < itemCount; ++i)
    {
        this->items[i] = items[i];
        pickedUpCounts.push_back(0);
    }
}

PickupTask::~PickupTask()
{
    delete[] items;
}

unsigned int PickupTask::hasPickedUpWoodBlock(not_null_ptr<ItemInstance> item)
{
    Item* pickedUpItem = item->getItem();
    return pickedUpItem == Item::byBlock(Blocks::log) ||
           pickedUpItem == Item::byBlock(Blocks::log2) ||
           pickedUpItem == Item::byBlock(Blocks::stripped_log_spruce) ||
           pickedUpItem == Item::byBlock(Blocks::stripped_log_birch) ||
           pickedUpItem == Item::byBlock(Blocks::stripped_log_jungle) ||
           pickedUpItem == Item::byBlock(Blocks::stripped_log_acacia) ||
           pickedUpItem == Item::byBlock(Blocks::stripped_log_dark_oak) ||
           pickedUpItem == Item::byBlock(Blocks::stripped_log_oak);
}

void PickupTask::onTake(not_null_ptr<ItemInstance> item, unsigned int itemCount,
                        unsigned int exactCount)
{
    bool countWood = false;
    unsigned int i;

    for (i = 0; i < this->itemCount; ++i)
    {
        if (item->getItem() != items[i])
            continue;

        if (items[i] == Items::potion && effect != NULL)
        {
            if (PotionUtils::hasEffect(item, effect))
                completed = true;
        }
        else if (hasPickedUpWoodBlock(item) && auxValue == -1)
        {
            pickedUpCounts[i] = itemCount;
            countWood = true;
        }
        else if (auxValue == -1 || itemCount < requiredCount)
        {
            if (auxValue != item->getAuxValue() || exactCount < requiredCount)
                continue;
            completed = true;
        }
        else
        {
            completed = true;
        }
    }

    if (hasPickedUpWoodBlock(item) && auxValue == -1)
        countWood = true;

    if (countWood)
    {
        unsigned int total = 0;
        for (i = 0; i < pickedUpCounts.size(); ++i)
            total += pickedUpCounts[i];
        if (total >= requiredCount)
            completed = true;
    }
}

bool PickupTask::isCompleted()
{
    return completed;
}
