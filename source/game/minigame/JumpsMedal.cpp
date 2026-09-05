#include "game/minigame/JumpsMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

JumpsMedal::~JumpsMedal()
{
}

double JumpsMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->getTotalJumpCount()));
}
