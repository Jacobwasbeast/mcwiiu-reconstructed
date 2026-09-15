#include "game/minigame/SprintingMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

SprintingMedal::~SprintingMedal() = default;

double SprintingMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->getTotalSprintDistance()));
}
