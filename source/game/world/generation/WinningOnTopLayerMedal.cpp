#include "game/world/generation/WinningOnTopLayerMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

WinningOnTopLayerMedal::~WinningOnTopLayerMedal()
{
}

double WinningOnTopLayerMedal::GetScore(PlayerStats* playerStats) const
{
    unsigned int count = 0;
    unsigned int round = 0;
    unsigned int totalRounds = playerStats->getTotalRounds();

    if (totalRounds != 0)
    {
        do
        {
            PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
            if (roundStats->mDeathLayer != PlayerStats::RoundStats::LAYERS_UNUSED && roundStats->mWon && roundStats->mFinalLayer == 0)
            {
                ++count;
            }

            ++round;
            totalRounds = playerStats->getTotalRounds();
        }
        while (round < totalRounds);
    }

    return static_cast<float>(static_cast<int>(count));
}
