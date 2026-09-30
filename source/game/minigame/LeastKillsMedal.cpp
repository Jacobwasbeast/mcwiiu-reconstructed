#include "game/minigame/LeastKillsMedal.hpp"

#include "game/core/TotalKillsStatTracker.hpp"
#include "game/world/generation/PlayerStats.hpp"

double LeastKillsMedal::GetScore(PlayerStats* playerStats) const
{
    return 1.0f / static_cast<float>(playerStats->mTotalKills->getValue());
}
