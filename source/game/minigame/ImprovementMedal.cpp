#include "game/minigame/ImprovementMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

ImprovementMedal::ImprovementMedal(int name, float target, unsigned int weight)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false)
{
}

double ImprovementMedal::GetScore(PlayerStats* playerStats) const
{
    bool hasOnePointRound = false;
    unsigned int improvedRounds = 0;
    unsigned int round = 0;
    unsigned int totalRounds = playerStats->getTotalRounds();

    while (round < totalRounds)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        if (roundStats->mPoints == 1)
        {
            hasOnePointRound = true;
        }
        if (roundStats->mWon && hasOnePointRound)
        {
            hasOnePointRound = false;
            ++improvedRounds;
        }
        ++round;
        totalRounds = playerStats->getTotalRounds();
    }

    return static_cast<double>(static_cast<float>(improvedRounds));
}
