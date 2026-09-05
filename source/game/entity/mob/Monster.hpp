#ifndef GAME_ENTITY_MOB_MONSTER_HPP
#define GAME_ENTITY_MOB_MONSTER_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/PathfinderMob.hpp"

class BlockPos;
class DamageSource;
class Level;
class Player;
class SoundEvent;
class Entity;
#include "game/client/audio/SoundSource.hpp"

class Monster : public PathfinderMob
{
public:
    Monster(Level* level);
    virtual ~Monster();
    virtual unsigned long long GetType();
    virtual SoundSource::ESoundSource getSoundSource();
    virtual void aiStep();
    virtual void tick();
    virtual const SoundEvent* getSwimSound();
    virtual const SoundEvent* getSwimSplashSound();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getFallDamageSound(int distance);
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    virtual double getWalkTargetValue(const BlockPos& pos);
    virtual bool isDarkEnoughToSpawn();
    virtual bool checkSpawnRules();
    virtual void registerAttributes();
    virtual bool shouldDropExperience();
    virtual bool isPreventingPlayerRest(boost::shared_ptr<Player> player);
};

#endif
