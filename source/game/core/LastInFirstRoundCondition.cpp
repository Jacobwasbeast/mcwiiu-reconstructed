#include "game/core/LastInFirstRoundCondition.hpp"

#include "game/world/generation/PlayerStats.hpp"

class GameStats
{
public:
    PlayerStats* GetPlayerStats(const StatsUID& uid);
};

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
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(0);
        int lastPosition = roundStats->mCheckpointPadding[0];
        int numberOfPlayers = roundStats->mCheckpointPadding[1];
        if (numberOfPlayers > 1)
            return lastPosition == numberOfPlayers - 1;
    }

    return false;
}
