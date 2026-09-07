#ifndef GAME_CORE_WITCH_HPP
#define GAME_CORE_WITCH_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Monster.hpp"
#include "game/entity/mob/RangedAttackMob.hpp"

class DamageSource;
class DataFixerUpper;
class Level;
class LivingEntity;
class SoundEvent;

class Witch : public Monster, public RangedAttackMob
{
public:
    Witch(Level* level);
    virtual ~Witch();
    static void create(Level* level);
    virtual void performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor);
    virtual bool isAggressive();
    virtual void setAggressive(bool aggressive);
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void registerGoals();
    virtual void defineSynchedData();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void setUsingItem(bool usingItem);
    virtual bool isDrinkingPotion();
    virtual void registerAttributes();
    virtual void aiStep();
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual float getDamageAfterMagicAbsorb(const DamageSource* source, float damage);
    virtual const void* getDefaultLootTable();
    virtual double getEyeHeight();
    virtual unsigned long long GetType();
    static void DEATH_LOOT();

private:
    int mUsingTime;
};

#endif
