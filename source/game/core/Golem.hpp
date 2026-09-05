#ifndef GAME_CORE_GOLEM_HPP
#define GAME_CORE_GOLEM_HPP

#include "game/entity/mob/PathfinderMob.hpp"

class DamageSource;
class Level;
class SoundEvent;

class Golem : public PathfinderMob
{
public:
    Golem(Level* level);
    virtual ~Golem();
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual unsigned int getAmbientSoundInterval();
    virtual bool removeWhenFarAway();
};

#endif
