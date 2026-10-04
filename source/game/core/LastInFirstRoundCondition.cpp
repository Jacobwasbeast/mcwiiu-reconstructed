#include "game/core/LastInFirstRoundCondition.hpp"

#include "game/core/GameStats.hpp"
#include "game/world/generation/PlayerStats.hpp"

LastInFirstRoundCondition::LastInFirstRoundCondition()
    : MultiConditionMedal::MultiCondition(-1.0f, GameMedal::OPERATOR_EQUAL, false)
{
}

LastInFirstRoundCondition::~LastInFirstRoundCondition()
{
}

bool LastInFirstRoundCondition::canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats != NULL && playerStats->getTotalRounds() != 0)
    {
        int finishPosition = playerStats->getRoundStats(0)->mFinishPosition;
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(0);
        if (roundStats->mFinishPlayerCount > 1)
            return finishPosition == roundStats->mFinishPlayerCount - 1;
    }

    return false;
}
