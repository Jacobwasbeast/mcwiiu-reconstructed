#include "game/minigame/JumpsMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

double JumpsMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->getTotalJumpCount()));
}
