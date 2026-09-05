#ifndef GAME_CORE_PIG_HPP
#define GAME_CORE_PIG_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Animal.hpp"

class Block;
class BlockPos;
class CompoundTag;
class DataFixerUpper;
class DamageSource;
class EntityDataAccessor_Base;
class EntityTooltipDataHolder;
class Level;
class LightningBolt;
class SoundEvent;
class ItemInstance;

template <class T> class EntityDataAccessor;

class Pig : public Animal
{
public:
    Pig(Level* level);
    virtual ~Pig();

    static void create(Level* level);
    static void staticCtor();
    virtual void registerGoals();
    virtual boost::shared_ptr<Entity> getControllingPassenger();
    virtual void registerAttributes();
    virtual bool canBeControlledByRider();
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual void defineSynchedData();
    static void addDataWalkers(DataFixerUpper* fixer);
    bool hasSaddle();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void setSaddle(bool saddled);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual bool mobInteract(const boost::shared_ptr<Player>& player,
                             InteractionHand::EInteractionHand hand);
    virtual void die(const DamageSource* source);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual const void* getDefaultLootTable();
    virtual void thunderHit(const LightningBolt* lightning);
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual void travel(float xxa, float yya, float zza);
    bool boost();
    bool testboost();
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual bool isFood(const not_null_ptr<ItemInstance>& itemInstance);
    virtual bool removeWhenFarAway();
    virtual unsigned long long GetType();

private:
    static const EntityDataAccessor<bool>* DATA_SADDLE;
    static const EntityDataAccessor<int>* DATA_BOOST_TIME;
    bool mBoosting;
    int mBoostTime;
    int mBoostTotalTime;
    long long mBoostStartTime;
};

#endif
