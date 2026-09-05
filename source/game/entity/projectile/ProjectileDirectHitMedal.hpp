#ifndef GAME_ENTITY_PROJECTILE_PROJECTILEDIRECTHITMEDAL_HPP
#define GAME_ENTITY_PROJECTILE_PROJECTILEDIRECTHITMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class ProjectileDirectHitMedal : public GameMedal
{
public:
    virtual ~ProjectileDirectHitMedal();

    virtual double GetScore(PlayerStats* playerStats) const;
};

#endif
