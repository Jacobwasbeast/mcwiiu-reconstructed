#include "game/entity/mob/HorseChoiceTask.hpp"

#include "game/entity/mob/Entity.hpp"

HorseChoiceTask::HorseChoiceTask(Tutorial* tutorial, int horseDescriptionIdValue,
                                 int donkeyDescriptionIdValue, int muleDescriptionIdValue,
                                 int fourthDescriptionIdValue, int promptId,
                                 bool preCompletionEnabled,
                                 int firstInput, int secondInput,
                                 eTutorial_CompletionAction completionAction,
                                 ETelemetryChallenges telemetryChallenge)
    : ChoiceTask(tutorial, -1, promptId, preCompletionEnabled, firstInput, secondInput,
                 completionAction, telemetryChallenge),
      mState(0), entityType(0), horseDescriptionId(horseDescriptionIdValue),
      donkeyDescriptionId(donkeyDescriptionIdValue), muleDescriptionId(muleDescriptionIdValue),
      fourthDescriptionId(fourthDescriptionIdValue)
{
}

HorseChoiceTask::~HorseChoiceTask()
{
}

int HorseChoiceTask::getDescriptionId()
{
    if (entityType == 0x2D698002 && mState == 0)
    {
        return muleDescriptionId;
    }
    if (entityType == 0x2D698001 && mState == 0)
    {
        return donkeyDescriptionId;
    }
    if (entityType == 0x2D698003 && mState == 0)
    {
        return fourthDescriptionId;
    }
    if (entityType == 0x2D690001 && mState == 0)
    {
        return horseDescriptionId;
    }
    return -1;
}

void HorseChoiceTask::onLookAtEntity(boost::shared_ptr<Entity> entity)
{
    if (mState != 0 || entityType != 0)
    {
        return;
    }

    if (entity->GetType() != 0x2D690001ULL && entity->GetType() != 0x2D698001ULL &&
        entity->GetType() != 0x2D698002ULL && entity->GetType() != 0x2D698003ULL)
    {
        return;
    }

    entityType = entity->GetType();
}
