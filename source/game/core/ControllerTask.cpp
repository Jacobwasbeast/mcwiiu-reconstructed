#include "game/core/ControllerTask.hpp"

#include "game/core/GameSettings.hpp"
#include "game/core/InputConstraint.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"

ControllerTask::ControllerTask(Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                               bool showMinimumTime, int* mappings, unsigned int mappingCount,
                               int* completionIdValues, int completionIdValueCount,
                               int* alternateMappings, unsigned int alternateMappingCount)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, NULL, showMinimumTime, true, true),
      useAlternateMappings(false)
{
    unsigned int i;

    for (i = 0; i < mappingCount; ++i)
    {
        constraints.push_back(new InputConstraint(mappings[i]));
        constrainedMappings[mappings[i]] = false;
    }

    if (alternateMappingCount != 0)
    {
        useAlternateMappings = true;

        for (i = 0; i < alternateMappingCount; ++i)
        {
            inputMappings[alternateMappings[i]] = false;
        }
    }

    completionIds = new int[completionIdValueCount];

    for (int completionId = 0; completionId < completionIdValueCount; ++completionId)
    {
        completionIds[completionId] = completionIdValues[completionId];
    }

    completionIdCount = completionIdValueCount;
    completionMask = 0;
}

ControllerTask::~ControllerTask()
{
    delete[] completionIds;
}

unsigned int ControllerTask::CompletionMaskIsValid()
{
    for (int i = 0; i < completionIdCount; ++i)
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
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::unordered_map<int, bool>* mappings = &constrainedMappings;

    if (useAlternateMappings
        && GameSettings::GetGameSettings(minecraft->mPlayer->mUserIndex,
                                         static_cast<eGameSetting>(9)) != 0)
    {
        mappings = &inputMappings;
    }

    unsigned int mappingIndex = 0;

    for (boost::unordered_map<int, bool>::iterator mapping = mappings->begin();
         mapping != mappings->end(); ++mapping, ++mappingIndex)
    {
        if (mapping->second)
        {
            continue;
        }

        if (g_pInput->GetValue(minecraft->mPlayer->mUserIndex,
                               static_cast<unsigned char>(mapping->first), false) == 0)
        {
            result = false;
            continue;
        }

        mapping->second = true;
        completionMask |= 1 << (mappingIndex & 0x3f);
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
