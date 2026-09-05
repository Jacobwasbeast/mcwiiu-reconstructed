#include "game/core/ControllerTask.hpp"

#include "game/core/GameSettings.hpp"
#include "game/core/InputConstraint.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/Tutorial.hpp"
#include "platform/wiiu/CInput.hpp"

ControllerTask::ControllerTask(Tutorial* tutorialValue, int descriptionIdValue,
                               bool preCompletionEnabledValue, bool showMinimumTimeValue,
                               int* constrainedMappingsValue,
                               unsigned int constrainedMappingCountValue,
                               int* completionIdsValue, int completionIdCountValue,
                               int* inputMappingsValue, unsigned int inputMappingCountValue)
    : TutorialTask(tutorialValue, descriptionIdValue, preCompletionEnabledValue, NULL,
                   showMinimumTimeValue, true, true),
      useAlternateMappings(false), completionMask(0), completionIds(NULL),
      completionIdCount(0)
{
    unsigned int i;

    for (i = 0; i + 1 < constrainedMappingCountValue; i += 2)
    {
        constraints.push_back(new InputConstraint(constrainedMappingsValue[i]));
        constrainedMappings[constrainedMappingsValue[i]] = false;
        constraints.push_back(new InputConstraint(constrainedMappingsValue[i + 1]));
        constrainedMappings[constrainedMappingsValue[i + 1]] = false;
    }
    if ((constrainedMappingCountValue & 1) != 0)
    {
        constraints.push_back(new InputConstraint(constrainedMappingsValue[i]));
        constrainedMappings[constrainedMappingsValue[i]] = false;
    }

    if (inputMappingCountValue != 0)
    {
        useAlternateMappings = true;
        for (i = 0; i < inputMappingCountValue; ++i)
        {
            inputMappings[inputMappingsValue[i]] = false;
        }
    }

    completionIds = new int[completionIdCountValue];
    for (i = 0; i < static_cast<unsigned int>(completionIdCountValue); ++i)
    {
        completionIds[i] = completionIdsValue[i];
    }
    completionIdCount = static_cast<unsigned int>(completionIdCountValue);
    completionMask = 0;
}

ControllerTask::~ControllerTask()
{
    delete[] completionIds;
}

unsigned int ControllerTask::CompletionMaskIsValid()
{
    unsigned int i;

    for (i = 0; i < completionIdCount; ++i)
    {
        if (completionMask == static_cast<unsigned int>(completionIds[i]))
        {
            return 1;
        }
    }
    return 0;
}

bool ControllerTask::isCompleted()
{
    if (completed)
    {
        return true;
    }

    bool result = true;
    boost::unordered_map<int, bool>* mappings = &constrainedMappings;
    Minecraft::GetInstance();
    int playerIndex = tutorial->playerId;

    if (useAlternateMappings && GameSettings::GetGameSettings(playerIndex, (eGameSetting)9) != 0)
    {
        mappings = &inputMappings;
    }

    boost::unordered_map<int, bool>::iterator mapping = mappings->begin();
    unsigned int mappingIndex = 0;
    while (mapping != mappings->end())
    {
        if (!mapping->second)
        {
            if (g_pInput->GetValue(playerIndex, (unsigned char)mapping->first, false) == 0)
            {
                result = false;
                ++mapping;
                ++mappingIndex;
                continue;
            }
            mapping->second = true;
            completionMask |= 1 << (mappingIndex & 0x3f);
        }
        ++mapping;
        ++mappingIndex;
    }

    if (completionIds != NULL && CompletionMaskIsValid())
    {
        completed = true;
        return true;
    }
    completed = result;
    return result;
}

void ControllerTask::setAsCurrentTask(bool currentTask)
{
    TutorialTask::setAsCurrentTask(currentTask);
    enableConstraints(!currentTask, false);
}
