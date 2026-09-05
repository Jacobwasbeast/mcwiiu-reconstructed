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
      entityType(0), horseDescriptionId(horseDescriptionIdValue),
      donkeyDescriptionId(donkeyDescriptionIdValue), muleDescriptionId(muleDescriptionIdValue),
      fourthDescriptionId(fourthDescriptionIdValue)
{
}

HorseChoiceTask::~HorseChoiceTask()
{
}

int HorseChoiceTask::getDescriptionId()
{
    if (entityType == 0x2D698002ULL)
    {
        return muleDescriptionId;
    }
    if (entityType == 0x2D698001ULL)
    {
        return donkeyDescriptionId;
    }
    if (entityType == 0x2D698003ULL)
    {
        return fourthDescriptionId;
    }
    if (entityType == 0x2D690001ULL)
    {
        return horseDescriptionId;
    }
    return -1;
}

void HorseChoiceTask::onLookAtEntity(boost::shared_ptr<Entity> entity)
{
    if (entityType != 0)
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
