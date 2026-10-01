#include "game/core/ProcedureCompoundTask.hpp"

#include "game/item/not_null_ptr.hpp"
#include "game/minigame/TutorialTask.hpp"
#include "game/world/block/BlockPos.hpp"

ProcedureCompoundTask::~ProcedureCompoundTask()
{
    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        if (*it != NULL)
        {
            (*it)->enableConstraints(false, false);
        }
    }
}

void ProcedureCompoundTask::AddTask(TutorialTask* task)
{
    if (task != NULL)
    {
        tasks.push_back(task);
    }
}

int ProcedureCompoundTask::getDescriptionId()
{
    if (completed)
    {
        return -1;
    }

    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        TutorialTask* task = *it;
        if (!task->isCompleted())
        {
            task->setAsCurrentTask(true);
            return task->getDescriptionId();
        }
        if (task->getCompletionAction() == eTutorial_CompletionAction_CompleteState)
        {
            completed = true;
            break;
        }
    }

    return -1;
}

int ProcedureCompoundTask::getPromptId()
{
    if (completed)
    {
        return -1;
    }

    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        TutorialTask* task = *it;
        if (!task->isCompleted())
        {
            return task->getPromptId();
        }
    }

    return -1;
}

bool ProcedureCompoundTask::isCompleted()
{
    bool result = true;

    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        TutorialTask* task = *it;
        if (!result)
        {
            task->setAsCurrentTask(false);
            continue;
        }
        if (!task->isCompleted())
        {
            task->setAsCurrentTask(true);
            result = false;
            continue;
        }
        if (task->getCompletionAction() == eTutorial_CompletionAction_CompleteState)
            break;
    }

    if (result)
    {
        for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
        {
            (*it)->enableConstraints(false, false);
        }
    }

    completed = result;
    return result;
}

void ProcedureCompoundTask::onCrafted(not_null_ptr<ItemInstance> item)
{
    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        (*it)->onCrafted(item);
    }
}

void ProcedureCompoundTask::handleUIInput(int input)
{
    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        (*it)->handleUIInput(input);
    }
}

void ProcedureCompoundTask::setAsCurrentTask(bool currentTask)
{
    bool current = true;
    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        TutorialTask* task = *it;
        if (current)
        {
            if (!task->isCompleted())
            {
                task->setAsCurrentTask(true);
                current = false;
            }
        }
        else
        {
            task->setAsCurrentTask(false);
        }
    }
}

bool ProcedureCompoundTask::ShowMinimumTime()
{
    if (completed)
    {
        return false;
    }

    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        TutorialTask* task = *it;
        if (!task->isCompleted())
        {
            return task->ShowMinimumTime();
        }
    }

    return false;
}

bool ProcedureCompoundTask::hasBeenActivated()
{
    if (completed)
    {
        return true;
    }

    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        TutorialTask* task = *it;
        if (!task->isCompleted())
        {
            return task->hasBeenActivated();
        }
    }

    return false;
}

void ProcedureCompoundTask::setShownForMinimumTime()
{
    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        TutorialTask* task = *it;
        if (!task->isCompleted())
        {
            task->setShownForMinimumTime();
            return;
        }
    }
}

bool ProcedureCompoundTask::AllowFade()
{
    if (completed)
    {
        return true;
    }

    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        TutorialTask* task = *it;
        if (!task->isCompleted())
        {
            return task->AllowFade();
        }
    }

    return true;
}

void ProcedureCompoundTask::useItemOn(Level* level, not_null_ptr<ItemInstance> item,
                                      const BlockPos& pos, bool cancelled)
{
    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        (*it)->useItemOn(level, item, pos, cancelled);
    }
}

void ProcedureCompoundTask::useItem(not_null_ptr<ItemInstance> item, bool cancelled)
{
    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        (*it)->useItem(item, cancelled);
    }
}

void ProcedureCompoundTask::onTake(not_null_ptr<ItemInstance> item, unsigned int itemCount,
                                   unsigned int exactCount)
{
    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        (*it)->onTake(item, itemCount, exactCount);
    }
}

void ProcedureCompoundTask::onStateChange(eTutorial_State state)
{
    for (std::vector<TutorialTask*>::iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        (*it)->onStateChange(state);
    }
}
