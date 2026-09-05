#ifndef GAME_CORE_BAT_HPP
#define GAME_CORE_BAT_HPP

#include "game/core/AmbientCreature.hpp"

class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class Level;
class SoundEvent;

template <class T> class EntityDataAccessor;

class Bat : public AmbientCreature
{
public:
    Bat(Level* level);
    virtual ~Bat();

    static void staticCtor();
    virtual void setResting(bool resting);
    virtual void defineSynchedData();
    virtual float getSoundVolume();
    virtual float getVoicePitch();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual bool isPushable(boost::shared_ptr<Entity> entity);
    virtual void doPush(boost::shared_ptr<Entity> entity);
    virtual void pushEntities();
    virtual void registerAttributes();
    virtual bool isResting();
    virtual void tick();
    virtual void customServerAiStep();
    virtual bool makeStepSound();
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual void checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos);
    virtual bool isIgnoringBlockTriggers();
    virtual bool hurt(const DamageSource* source, float damage);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual bool checkSpawnRules();
    virtual double getEyeHeight();
    virtual const void* getDefaultLootTable();
    virtual unsigned long long GetType();
    static void create(Level* level);

private:
    static const EntityDataAccessor<unsigned char>* DATA_ID_FLAGS;
    BlockPos* mTargetPosition;
};

#endif
