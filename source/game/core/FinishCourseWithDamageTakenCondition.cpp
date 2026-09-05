#include "game/core/FinishCourseWithDamageTakenCondition.hpp"

#include "game/core/GameStats.hpp"
#include "game/world/generation/PlayerStats.hpp"

FinishCourseWithDamageTakenCondition::FinishCourseWithDamageTakenCondition(float target)
    : MultiConditionMedal::MultiCondition(target, GameMedal::OPERATOR_EQUAL, false)
{
}

FinishCourseWithDamageTakenCondition::~FinishCourseWithDamageTakenCondition()
{
}

bool FinishCourseWithDamageTakenCondition::canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats != NULL)
    {
        unsigned int round = 0;
        int totalRounds = playerStats->getTotalRounds();
        if (totalRounds != 0)
        {
            do
            {
                if (playerStats->getReachedEnd(round) != 0 && playerStats->getDamageTaken(round) == static_cast<double>(mTarget))
                {
                    return true;
                }

                ++round;
                totalRounds = playerStats->getTotalRounds();
            }
            while (round < static_cast<unsigned int>(totalRounds));
        }
    }

    return false;
}
