#include "game/world/block/BlocksDestroyedFromBelowMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

BlocksDestroyedFromBelowMedal::~BlocksDestroyedFromBelowMedal()
{
}

float BlocksDestroyedFromBelowMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<float>(playerStats->getTotalBlocksDestroyed());
}
