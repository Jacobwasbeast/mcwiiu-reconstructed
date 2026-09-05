#include "game/entity/projectile/ProjectileDirectHitMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

ProjectileDirectHitMedal::~ProjectileDirectHitMedal()
{
}

double ProjectileDirectHitMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<float>(static_cast<unsigned int>(playerStats->mSnowballDirectHits));
}
