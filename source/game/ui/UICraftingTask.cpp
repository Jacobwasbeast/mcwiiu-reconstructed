#include "game/ui/UICraftingTask.hpp"

#include "game/item/Item.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/IUIScene_CraftingMenu.hpp"

UICraftingTask::UICraftingTask(Tutorial* tutorial, int descriptionId,
                               Recipe::_eGroupType group, bool preCompletionEnabled,
                               std::vector<TutorialConstraint*>* constraints,
                               bool showMinimumTime, bool allowFade, bool repeatMessage)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, constraints,
                   showMinimumTime, allowFade, repeatMessage)
{
    this->group = group;
    taskType = 0;
}

UICraftingTask::UICraftingTask(Tutorial* tutorial, int descriptionId, Item* item,
                               bool preCompletionEnabled,
                               std::vector<TutorialConstraint*>* constraints,
                               bool showMinimumTime, bool allowFade, bool repeatMessage)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, constraints,
                   showMinimumTime, allowFade, repeatMessage)
{
    this->item = item;
    taskType = 1;
}

UICraftingTask::~UICraftingTask()
{
}

bool UICraftingTask::isCompleted()
{
    UIScene* scene = tutorial->currentScene;
    if (taskType == 0)
    {
        if (scene == NULL)
        {
            return false;
        }

        IUIScene_CraftingMenu* craftingMenu = (IUIScene_CraftingMenu*)scene;
        return craftingMenu->getCurrentGroup() == group;
    }

    if (taskType != 1 || scene == NULL)
    {
        return false;
    }

    IUIScene_CraftingMenu* craftingMenu = (IUIScene_CraftingMenu*)scene;
    return craftingMenu->isItemSelected(Item::getId(item));
}
