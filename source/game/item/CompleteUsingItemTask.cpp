#include "game/item/CompleteUsingItemTask.hpp"

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"

CompleteUsingItemTask::CompleteUsingItemTask(Tutorial* tutorial, int descriptionId,
                                             Item** items, unsigned int itemCount,
                                             bool preCompletionEnabled)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, NULL, false, true, true),
      items(new Item*[itemCount]), itemCount(itemCount)
{
    unsigned int index = 0;
    while (index < itemCount)
    {
        this->items[index] = items[index];
        ++index;
    }
}

CompleteUsingItemTask::~CompleteUsingItemTask()
{
    delete[] items;
}

bool CompleteUsingItemTask::isCompleted()
{
    return completed;
}

void CompleteUsingItemTask::completeUsingItem(not_null_ptr<ItemInstance> item)
{
    if ((hasBeenActivated() || isPreCompletionEnabled()) && itemCount > 0)
    {
        unsigned int index = 0;
        while (index < itemCount)
        {
            if (item->getItem() == items[index])
            {
                completed = true;
                return;
            }
            ++index;
        }
    }
}
