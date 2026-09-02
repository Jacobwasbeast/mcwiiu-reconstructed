#include "game/minigame/TutorialTask.hpp"

#include <cstddef>

#include "game/core/GameModeConstraints.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/minigame/TutorialConstraint.hpp"

TutorialTask::TutorialTask(Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                           std::vector<TutorialConstraint*>* constraints, bool showMinimumTime,
                           bool allowFade, bool repeatMessage)
    : descriptionId(descriptionId),
      promptId(-1),
      tutorial(tutorial),
      preCompletionEnabled(preCompletionEnabled),
      activated(false),
      allowFade(allowFade),
      repeatMessage(repeatMessage),
      showMinimumTime(showMinimumTime),
      completed(false),
      shownForMinimumTime(false),
      constraintsEnabled(false)
{
    if (constraints != NULL)
    {
        for (std::vector<TutorialConstraint*>::iterator constraint = constraints->begin();
             constraint < constraints->end(); ++constraint)
        {
            this->constraints.push_back(*constraint);
        }

        delete constraints;
    }

    tutorial->addMessage(descriptionId, false, 3);
}

TutorialTask::~TutorialTask()
{
    enableConstraints(false, false);

    for (std::vector<TutorialConstraint*>::iterator it = constraints.begin();
         it < constraints.end(); ++it)
    {
        TutorialConstraint* constraint = *it;

        if (!constraint->removalPending)
        {
            delete constraint;
        }
        else
        {
            constraint->deleteWhenRemoved = true;
        }
    }
}

int TutorialTask::getDescriptionId()
{
    return descriptionId;
}

int TutorialTask::getPromptId()
{
    return promptId;
}

eTutorial_CompletionAction TutorialTask::getCompletionAction()
{
    return eTutorial_CompletionAction_None;
}

bool TutorialTask::isPreCompletionEnabled()
{
    return preCompletionEnabled;
}

void TutorialTask::taskCompleted()
{
    if (!constraintsEnabled)
    {
        return;
    }

    enableConstraints(false, false);
}

void TutorialTask::enableConstraints(bool enable, bool delayed)
{
    if (enable)
    {
        if (!constraintsEnabled)
        {
            for (std::vector<TutorialConstraint*>::iterator constraint = constraints.begin();
                 constraint != constraints.end(); ++constraint)
            {
                tutorial->gameModeConstraints->AddConstraint(*constraint, false);
            }

            constraintsEnabled = true;
        }
    }
    else if (constraintsEnabled || !delayed)
    {
        for (std::vector<TutorialConstraint*>::iterator constraint = constraints.begin();
             constraint != constraints.end(); ++constraint)
        {
            tutorial->gameModeConstraints->RemoveConstraint(*constraint, delayed);
        }

        constraintsEnabled = false;
    }
}

void TutorialTask::setAsCurrentTask(bool currentTask)
{
    activated = currentTask;
}

void TutorialTask::setShownForMinimumTime()
{
    shownForMinimumTime = true;
}

bool TutorialTask::hasBeenActivated()
{
    return activated;
}

bool TutorialTask::AllowFade()
{
    return allowFade;
}

bool TutorialTask::ShowMinimumTime()
{
    return showMinimumTime;
}

void TutorialTask::useItemOn(Level* level, not_null_ptr<ItemInstance> item, const BlockPos& pos,
                             bool cancelled)
{
}

void TutorialTask::useItem(not_null_ptr<ItemInstance> item, bool cancelled)
{
}

void TutorialTask::completeUsingItem(not_null_ptr<ItemInstance> item)
{
}

void TutorialTask::handleUIInput(int input)
{
}

void TutorialTask::onCrafted(not_null_ptr<ItemInstance> item)
{
}

void TutorialTask::onEquip(not_null_ptr<ItemInstance> item)
{
}

void TutorialTask::onTake(not_null_ptr<ItemInstance> item, unsigned int itemCount,
                          unsigned int exactCount)
{
}

void TutorialTask::onTakeFromAnvil(not_null_ptr<ItemInstance> item)
{
}

void TutorialTask::onStateChange(eTutorial_State state)
{
}

void TutorialTask::onEffectChanged(MobEffect* effect, bool added)
{
}

void TutorialTask::onLookAtEntity(boost::shared_ptr<Entity> entity)
{
}

void TutorialTask::onRideEntity(boost::shared_ptr<Entity> entity)
{
}
