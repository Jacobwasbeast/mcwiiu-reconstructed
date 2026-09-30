#include "game/minigame/ClimbingMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

double ClimbingMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->getTotalClimbDistance()));
}
