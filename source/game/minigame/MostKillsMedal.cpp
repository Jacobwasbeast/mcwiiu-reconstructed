#include "game/minigame/MostKillsMedal.hpp"

#include "game/core/TotalKillsStatTracker.hpp"
#include "game/world/generation/PlayerStats.hpp"

MostKillsMedal::~MostKillsMedal()
{
}

double MostKillsMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->mTotalKills->getValue()));
}
