#include "game/core/StatTask.hpp"

#include "game/core/Minecraft.hpp"
#include "game/core/Stat.hpp"
#include "game/core/StatsCounter.hpp"
#include "platform/wiiu/CProfile.hpp"

StatTask::StatTask(Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                   Stat* stat, int requiredValue)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, NULL, false, true, true),
      mStat(stat), mRequiredValue(0)
{
    int playerIndex = gProfile->GetPrimaryPad();
    mRequiredValue = Minecraft::GetInstance()->mStats[playerIndex]->getTotalValue(mStat) + requiredValue;
}

StatTask::~StatTask() = default;

bool StatTask::isCompleted()
{
    if (completed)
    {
        return true;
    }

    int playerIndex = gProfile->GetPrimaryPad();
    completed = Minecraft::GetInstance()->mStats[playerIndex]->getTotalValue(mStat) >= mRequiredValue;
    return completed;
}
