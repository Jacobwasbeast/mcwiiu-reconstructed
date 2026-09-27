#include "game/entity/mob/HorseChoiceTask.hpp"
#include "game/core/EntityType.hpp"

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
    if (entityType == EntityType::MULE)
    {
        return muleDescriptionId;
    }
    if (entityType == EntityType::DONKEY)
    {
        return donkeyDescriptionId;
    }
    if (entityType == EntityType::LLAMA)
    {
        return fourthDescriptionId;
    }
    if (entityType == EntityType::HORSE)
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

    if (entity->GetType() != EntityType::HORSE && entity->GetType() != EntityType::DONKEY &&
        entity->GetType() != EntityType::MULE && entity->GetType() != EntityType::LLAMA)
    {
        return;
    }

    entityType = entity->GetType();
}
