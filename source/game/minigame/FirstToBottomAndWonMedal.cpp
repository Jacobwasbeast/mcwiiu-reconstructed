#include "game/minigame/FirstToBottomAndWonMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

FirstToBottomAndWonMedal::~FirstToBottomAndWonMedal()
{
}

double FirstToBottomAndWonMedal::GetScore(PlayerStats* playerStats) const
{
    unsigned int count = 0;

    for (unsigned int round = 0; round < playerStats->getTotalRounds(); round++)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        if (roundStats->mLayer != -2 && roundStats->mLives != 0 && roundStats->mWon)
        {
            count++;
        }
    }

    return static_cast<double>(static_cast<float>(count));
}
