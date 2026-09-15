#include "game/core/InfoTask.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/core/InputConstraint.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"

extern ConsoleUIController gConsoleUIController;

InfoTask::InfoTask(Tutorial* tutorial, int descriptionId, int promptId, bool addConstraint,
                   int completionId, ETelemetryChallenges telemetryChallenge)
    : TutorialTask(tutorial, descriptionId, false, NULL, true, false, false),
      completedInputs(), telemetryChallenge(telemetryChallenge)
{
    if (addConstraint)
    {
        constraints.push_back(new InputConstraint(completionId));
    }

    completedInputs[completionId] = false;
    this->promptId = promptId;
    tutorial->addMessage(promptId, false, 3);
}

InfoTask::~InfoTask()
{
}

void InfoTask::sendTelemetry()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (telemetryChallenge != eTelemetryChallenges_None)
    {
        if (telemetryChallenge == static_cast<ETelemetryChallenges>(3))
        {
            tutorial->getCompleted(388);
            tutorial->setCompleted(388);
        }

        telemetryManagerInstance->RecordEnemyKilledOrOvercome(
            minecraft->mPlayer->mUserIndex, 0, 0, 0, 0, 0, 0, telemetryChallenge);
    }
}

bool InfoTask::isCompleted()
{
    if (completed)
    {
        return true;
    }

    if (!activated || !shownForMinimumTime)
    {
        return false;
    }

    if (!gConsoleUIController.GetMenuDisplayed(tutorial->playerId))
    {
        boost::unordered_map<int, bool>::iterator input = completedInputs.begin();
        while (input != completedInputs.end())
        {
            if (!input->second)
            {
                if (g_pInput->GetValue(tutorial->playerId, (unsigned char)input->first, false))
                {
                    input->second = true;
                }
                else
                {
                    return false;
                }
            }
            ++input;
        }
    }
    else
    {
        boost::unordered_map<int, bool>::iterator input = completedInputs.begin();
        while (input != completedInputs.end())
        {
            if (!input->second)
            {
                return false;
            }
            ++input;
        }
    }

    sendTelemetry();
    enableConstraints(false, true);
    completed = true;
    return true;
}

int InfoTask::getPromptId()
{
    if (!shownForMinimumTime)
    {
        return -1;
    }
    return promptId;
}

void InfoTask::setAsCurrentTask(bool currentTask)
{
    enableConstraints(currentTask, false);
    TutorialTask::setAsCurrentTask(currentTask);
}

void InfoTask::handleUIInput(int input)
{
    if (!preCompletionEnabled || completedInputs.empty())
    {
        return;
    }

    boost::unordered_map<int, bool>::iterator current = completedInputs.begin();
    while (current != completedInputs.end())
    {
        if (current->first == input)
        {
            current->second = true;
        }
        ++current;
    }
}
