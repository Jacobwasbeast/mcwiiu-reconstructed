#include "game/core/TimeSpentCondition.hpp"

#include <algorithm>
#include <climits>

#include "game/world/generation/PlayerStats.hpp"

TimeSpentCondition::TimeSpentCondition(float target, eTimeSpentCategory category,
    GameMedal::EOperator op, bool bestOnly)
    : MultiConditionMedal::MultiCondition(target, op, bestOnly)
    , mCategory(category)
{
}

TimeSpentCondition::~TimeSpentCondition()
{
}

double TimeSpentCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    unsigned int score = mBestOnly ? UINT_MAX : 0;
    for (unsigned int round = 0; round < static_cast<unsigned int>(playerStats->getTotalRounds()); ++round)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        if (mCategory != eTimeSpentCategory_BetweenCheckpoints)
        {
            const unsigned int timeSpent = roundStats->mTimeSpent[mCategory];
            score = mBestOnly ? std::min(score, timeSpent) : std::max(score, timeSpent);
            continue;
        }

        if (roundStats->mLastCheckpointID == -1)
            continue;

        int previousTime = 0;
        for (unsigned int checkpoint = 0; checkpoint < static_cast<unsigned int>(roundStats->mLastCheckpointID + 1); ++checkpoint)
        {
            if (roundStats->mCheckpointTimes.find(checkpoint) == roundStats->mCheckpointTimes.end())
                continue;
            const unsigned int split = roundStats->mCheckpointTimes.at(checkpoint) - previousTime;
            previousTime = roundStats->mCheckpointTimes.at(checkpoint);
            score = mBestOnly ? std::min(score, split) : std::max(score, split);
        }
    }
    return static_cast<float>(score);
}
