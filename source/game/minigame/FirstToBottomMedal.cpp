#include "game/minigame/FirstToBottomMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

FirstToBottomMedal::~FirstToBottomMedal()
{
}

double FirstToBottomMedal::GetScore(PlayerStats* playerStats) const
{
    unsigned int count = 0;

    for (unsigned int round = 0; round < playerStats->getTotalRounds(); ++round)
    {
        PlayerStats::RoundStats* stats = playerStats->getRoundStats(round);
        if (stats->mTimeSpent[0] != -2 && stats->mReachedEnd)
        {
            ++count;
        }
    }

    return static_cast<double>(static_cast<float>(count));
}
