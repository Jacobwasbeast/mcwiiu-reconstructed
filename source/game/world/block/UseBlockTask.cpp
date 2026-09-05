#include "game/world/block/UseBlockTask.hpp"

#include "game/world/Level.hpp"

UseBlockTask::UseBlockTask(Block* block, Tutorial* tutorial, int descriptionId,
                           bool preCompletionEnabled,
                           std::vector<TutorialConstraint*>* constraints,
                           bool showMinimumTime, bool allowFade,
                           bool repeatMessage)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, constraints,
                   showMinimumTime, allowFade, repeatMessage),
      mPos(), mBlock(block), mHasPosition(false)
{
}

UseBlockTask::~UseBlockTask()
{
}

bool UseBlockTask::isCompleted()
{
    return completed;
}

void UseBlockTask::useItemOn(Level* level, not_null_ptr<ItemInstance>,
                             const BlockPos& pos, bool cancelled)
{
    if (!cancelled && (preCompletionEnabled || activated)
        && (!mHasPosition || mPos.equals(pos))
        && level->getBlock(pos) == mBlock)
    {
        completed = true;
    }
}
