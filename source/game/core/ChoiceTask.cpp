#include "game/core/ChoiceTask.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/core/InputConstraint.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"

ChoiceTask::ChoiceTask(Tutorial* tutorial, int descriptionId, int promptId, bool preCompletionEnabled,
                       int firstInput, int secondInput,
                       eTutorial_CompletionAction completionAction,
                       ETelemetryChallenges telemetryChallenge)
    : TutorialTask(tutorial, descriptionId, false, NULL, true, false, false),
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

    if (telemetryChallenge == static_cast<ETelemetryChallenges>(1))
    {
        tutorial->getCompleted(386);
        tutorial->setCompleted(386);
    }
    else if (telemetryChallenge == static_cast<ETelemetryChallenges>(2))
    {
        tutorial->getCompleted(387);
        tutorial->setCompleted(387);
    }

    telemetryManagerInstance->RecordEnemyKilledOrOvercome(minecraft->mPlayer->mUserIndex, 0, 0, 0, 0, 0, 0,
                                                  telemetryChallenge);
}

bool ChoiceTask::isCompleted()
{
    Minecraft* minecraft = Minecraft::GetInstance();

    if (firstChoice || secondChoice)
    {
        sendTelemetry();
        enableConstraints(false, true);
        return true;
    }

    if (!gConsoleUIController.GetMenuDisplayed(tutorial->playerId))
    {
        if (minecraft->mLocalPlayers[tutorial->playerId]->isUnderLiquid(Material::WATER))
        {
            return false;
        }

        if (!firstChoice && g_pInput->GetValue(minecraft->mPlayer->mUserIndex,
                                               static_cast<unsigned char>(firstInput), false))
        {
            firstChoice = true;
        }

        if (!secondChoice && g_pInput->GetValue(minecraft->mPlayer->mUserIndex,
                                                static_cast<unsigned char>(secondInput), false))
        {
            secondChoice = true;
        }

        if (firstChoice || secondChoice)
        {
            sendTelemetry();
            enableConstraints(false, true);
        }
    }

    return firstChoice || secondChoice;
}

eTutorial_CompletionAction ChoiceTask::getCompletionAction()
{
    return secondChoice ? completionAction : eTutorial_CompletionAction_None;
}

int ChoiceTask::getPromptId()
{
    if (!shownForMinimumTime)
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
    if (!activated || !shownForMinimumTime)
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
