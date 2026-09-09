#include "game/minigame/FirstToBottomMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

FirstToBottomMedal::~FirstToBottomMedal()
{
}

double FirstToBottomMedal::GetScore(PlayerStats* playerStats) const
{
    int count = 0;

    for (unsigned int round = 0; round < playerStats->getTotalRounds(); ++round)
    {
        PlayerStats::RoundStats* stats = playerStats->getRoundStats(round);
        if (stats->mLayer != -2 && stats->mFinished)
        {
            ++count;
        }
    }

    return static_cast<double>(static_cast<float>(count));
}
