#include "game/core/AreaTask.hpp"

#include "game/minigame/TutorialConstraint.hpp"

AreaTask::AreaTask(eTutorial_State stateValue, Tutorial* tutorialValue,
                   std::vector<TutorialConstraint*>* constraintsValue, int descriptionId,
                   EAreaTaskCompletionStates completionStateValue)
    : TutorialTask(tutorialValue, descriptionId, false, constraintsValue, false, false, false),
      completionState(completionStateValue), state(stateValue)
{
    if (stateValue == eTutorial_State_None)
    {
        enableConstraints(true, false);
    }
}

AreaTask::~AreaTask()
{
}

bool AreaTask::isCompleted()
{
    if (completed)
    {
        return true;
    }

    bool result = false;
    if (completionState == CompletionStateOnStateChange)
    {
        result = hasBeenActivated();
    }
    else if (completionState == CompletionStateOnConstraints)
    {
        std::vector<TutorialConstraint*>::iterator constraint = constraints.begin();
        while (constraint != constraints.end())
        {
            if (!(*constraint)->isConstraintSatisfied(tutorial->playerId))
            {
                completed = false;
                return false;
            }
            ++constraint;
        }
        completed = true;
        return true;
    }

    completed = result;
    return result;
}

void AreaTask::setAsCurrentTask(bool currentTask)
{
    TutorialTask::setAsCurrentTask(currentTask);
    if (completionState == CompletionStateOnConstraints)
    {
        enableConstraints(currentTask, false);
    }
}

void AreaTask::onStateChange(eTutorial_State stateValue)
{
    if (completionState != CompletionStateOnStateChange)
    {
        return;
    }
    if (state != stateValue)
    {
        return;
    }
    enableConstraints(true, false);
}
