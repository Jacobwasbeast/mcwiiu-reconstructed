#include "game/world/generation/TimeInLayerCentreMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

TimeInLayerCentreMedal::~TimeInLayerCentreMedal()
{
}

double TimeInLayerCentreMedal::GetScore(PlayerStats* playerStats) const
{
    int total = 0;

    for (unsigned int round = 0; round < static_cast<unsigned int>(playerStats->getTotalRounds()); ++round)
    {
        total += playerStats->getRoundStats(round)->mLastCheckpointSequenceId;
    }

    return static_cast<double>(static_cast<float>(total));
}
