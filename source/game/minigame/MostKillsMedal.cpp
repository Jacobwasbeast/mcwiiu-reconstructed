#include "game/minigame/MostKillsMedal.hpp"

#include "game/core/StatTracker.hpp"
#include "game/world/generation/PlayerStats.hpp"

MostKillsMedal::~MostKillsMedal()
{
}

float MostKillsMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<float>(playerStats->mTotalKills->getValue());
}
