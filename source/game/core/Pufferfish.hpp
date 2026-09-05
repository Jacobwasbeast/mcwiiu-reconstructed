#ifndef GAME_CORE_PUFFERFISH_HPP
#define GAME_CORE_PUFFERFISH_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Fish.hpp"

class DamageSource;
class DataFixerUpper;
class Entity;
class Level;
class SoundEvent;

class Pufferfish : public Fish
{
public:
    enum PuffedState
    {
        PUFFED_STATE_SMALL = 0,
        PUFFED_STATE_MEDIUM = 1,
        PUFFED_STATE_LARGE = 2
    };

    Pufferfish(Level* level);
    virtual ~Pufferfish();

    static void create(Level* level);
    void setNextPuffedState(PuffedState state, int delay);
    void damageNearbyMobs();
    bool validatePuffedTypes(boost::shared_ptr<Entity> entity);
    void checkForDangerousMobs();
    virtual void baseTick();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual const SoundEvent* getFlopSound() const;
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const void* getDefaultLootTable();
    virtual unsigned long long GetType();
    static void addDataWalkers(DataFixerUpper* fixer);

private:
    PuffedState mPuffState;
    PuffedState mNextPuffState;
    int mPuffTransitionTime;
    int mPoisonDuration;
    float mDamage;
    float mSmallPuffRadius;
    float mLargePuffRadius;
    float mPuffSoundPitch;
    int mField778;
    int mField77c;
    int mField780;
    int mField784;
};

#endif
