#include "game/core/ChoiceTask.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/core/InputConstraint.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/block/Material.hpp"
#include "platform/wiiu/CInput.hpp"

class CTelemetryManager
{
public:
    unsigned int RecordPlayerDiedOrFailed(int playerId, int locationX, int locationY,
                                          int locationZ, int enemyTypeId, int playerWeaponId,
                                          int causeOfDeathId, ETelemetryChallenges challenges);
};

extern CTelemetryManager gTelemetryManager;
extern ConsoleUIController gConsoleUIController;

ChoiceTask::ChoiceTask(Tutorial* tutorial, int descriptionId, int promptId, bool preCompletionEnabled,
                       int firstInput, int secondInput,
                       eTutorial_CompletionAction completionAction,
                       ETelemetryChallenges telemetryChallenge)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, NULL, true, false, false),
      firstInput(firstInput), secondInput(secondInput), firstChoice(false), secondChoice(false),
      completionAction(completionAction), telemetryChallenge(telemetryChallenge)
{
    if (preCompletionEnabled)
    {
        constraints.push_back(new InputConstraint(firstInput));
        constraints.push_back(new InputConstraint(secondInput));
    }

    this->promptId = promptId;
    tutorial->addMessage(promptId, false, 3);
}

ChoiceTask::~ChoiceTask()
{
}

void ChoiceTask::sendTelemetry()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (telemetryChallenge == eTelemetryChallenges_None)
    {
        return;
    }

    int playerId = tutorial->playerId;
    if (telemetryChallenge == (ETelemetryChallenges)1)
    {
        tutorial->getCompleted(386);
        tutorial->setCompleted(386);
    }
    else if (telemetryChallenge == (ETelemetryChallenges)2)
    {
        tutorial->getCompleted(387);
        tutorial->setCompleted(387);
    }

    gTelemetryManager.RecordPlayerDiedOrFailed(playerId, 0, 0, 0, 0, 0, 0,
                                                telemetryChallenge);
    (void)minecraft;
}

bool ChoiceTask::isCompleted()
{
    if (firstChoice || secondChoice)
    {
        sendTelemetry();
        enableConstraints(false, true);
        return true;
    }

    Minecraft* minecraft = Minecraft::GetInstance();
    if (gConsoleUIController.GetMenuDisplayed(tutorial->playerId))
    {
        return false;
    }

    if (!firstChoice && g_pInput->GetValue(tutorial->playerId, (unsigned char)firstInput, false))
    {
        firstChoice = true;
    }
    if (!secondChoice && g_pInput->GetValue(tutorial->playerId, (unsigned char)secondInput, false))
    {
        secondChoice = true;
    }

    if (firstChoice || secondChoice)
    {
        sendTelemetry();
        enableConstraints(false, true);
    }

    return firstChoice || secondChoice;
}

eTutorial_CompletionAction ChoiceTask::getCompletionAction()
{
    return secondChoice ? completionAction : eTutorial_CompletionAction_None;
}

int ChoiceTask::getPromptId()
{
    if (!promptId)
    {
        return -1;
    }
    return promptId;
}

void ChoiceTask::setAsCurrentTask(bool currentTask)
{
    enableConstraints(currentTask, false);
    TutorialTask::setAsCurrentTask(currentTask);
}

void ChoiceTask::handleUIInput(int input)
{
    if (!activated || !promptId)
    {
        return;
    }
    if (input == firstInput)
    {
        firstChoice = true;
    }
    else if (input == secondInput)
    {
        secondChoice = true;
    }
}
