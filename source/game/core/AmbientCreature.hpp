#ifndef GAME_CORE_AMBIENTCREATURE_HPP
#define GAME_CORE_AMBIENTCREATURE_HPP

#include "game/entity/mob/Mob.hpp"

class AmbientCreature : public Mob
{
public:
    AmbientCreature(Level* level);
    virtual ~AmbientCreature();
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
};

#endif
