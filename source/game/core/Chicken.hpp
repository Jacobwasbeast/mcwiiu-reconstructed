#ifndef GAME_CORE_CHICKEN_HPP
#define GAME_CORE_CHICKEN_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Animal.hpp"

class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class Entity;
class ItemInstance;
class Level;
class SoundEvent;
class AgableMob;
template <class T> class not_null_ptr;

class Chicken : public Animal
{
public:
    Chicken(Level* level);
    virtual ~Chicken();
    static void addDataWalkers(DataFixerUpper* fixer);
    void _init();
    virtual void registerGoals();
    static void staticCtor();
    virtual double getEyeHeight();
    virtual void registerAttributes();
    virtual void aiStep();
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual const void* getDefaultLootTable();
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> parent);
    virtual bool isFood(const not_null_ptr<ItemInstance>& itemInstance);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool canBeControlledByRider();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual bool removeWhenFarAway();
    virtual void positionRider(const boost::shared_ptr<Entity>& passenger);
    virtual unsigned long long GetType();
    static void create(Level* level);

private:
    float mFlapTime;
    float mFlapSpeed;
    float mOFlapSpeed;
    float mOFlap;
    float mFlapping;
    int mTimeUntilNextEgg;
};

#endif
