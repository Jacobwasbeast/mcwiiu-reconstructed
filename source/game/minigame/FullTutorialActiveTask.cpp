#include "game/minigame/FullTutorialActiveTask.hpp"

FullTutorialActiveTask::FullTutorialActiveTask(Tutorial* tutorialValue,
                                               eTutorial_CompletionAction completionActionValue)
    : TutorialTask(tutorialValue, -1, false, NULL, false, false, false),
      completionAction(completionActionValue)
{
}

FullTutorialActiveTask::~FullTutorialActiveTask()
{
}

bool FullTutorialActiveTask::isCompleted()
{
    return completed;
}

eTutorial_CompletionAction FullTutorialActiveTask::getCompletionAction()
{
    if (!tutorial->enabled)
    {
        return eTutorial_CompletionAction_None;
    }
    return completionAction;
}
