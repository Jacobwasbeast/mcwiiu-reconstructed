#include "game/minigame/LongWayDownMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

LongWayDownMedal::~LongWayDownMedal()
{
}

double LongWayDownMedal::GetScore(PlayerStats* playerStats) const
{
    unsigned int count = 0;

    for (unsigned int round = 0; round < playerStats->getTotalRounds(); ++round)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        if (roundStats->mTimeSpent[0] == 0 && !roundStats->mActive)
            ++count;
    }

    return (double)(float)count;
}
