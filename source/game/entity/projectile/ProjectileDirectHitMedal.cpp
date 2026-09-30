#include "game/entity/projectile/ProjectileDirectHitMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

double ProjectileDirectHitMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<float>(playerStats->mSnowballDirectHits);
}
