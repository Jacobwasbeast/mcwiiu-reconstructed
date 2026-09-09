#include "game/world/block/BlocksDestroyedFromBelowMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

BlocksDestroyedFromBelowMedal::~BlocksDestroyedFromBelowMedal()
{
}

double BlocksDestroyedFromBelowMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->getTotalBlocksDestroyed()));
}
