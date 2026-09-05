#include "game/entity/projectile/HitsFromProjectileMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

HitsFromProjectileMedal::~HitsFromProjectileMedal()
{
}

double HitsFromProjectileMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(static_cast<unsigned int>(playerStats->mHitsFromProjectile)));
}
