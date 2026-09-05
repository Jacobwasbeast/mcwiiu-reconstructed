#ifndef GAME_CORE_CREEPER_HPP
#define GAME_CORE_CREEPER_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Monster.hpp"

class CompoundTag;
class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;
class LightningBolt;
class Entity;

class Creeper : public Monster
{
public:
    Creeper(Level* level);
    virtual ~Creeper();

    static void staticCtor();
    void _init();
    void registerGoals();
    virtual void registerAttributes();
    virtual int getMaxFallDistance();
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual void defineSynchedData();
    static void addDataWalkers(DataFixerUpper* fixer);
    bool isIgnited();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void ignite();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    int getSwellDir();
    void setSwellDir(int swellDir);
    bool spawnLingeringCloud();
    void explodeCreeper();
    virtual void tick();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    bool canDropMobsSkull();
    void increaseDroppedSkulls();
    virtual void die(const DamageSource* source);
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    bool isPowered();
    double getSwelling(float partialTicks);
    virtual const void* getDefaultLootTable();
    virtual void thunderHit(const LightningBolt* lightning);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual unsigned long long GetType();
    static void create(Level* level);

private:
    static const EntityDataAccessor<int>* DATA_SWELL_DIR;
    static const EntityDataAccessor<bool>* DATA_IS_POWERED;
    static const EntityDataAccessor<bool>* DATA_IS_IGNITED;

    int mOldSwell;
    int mSwell;
    int mMaxSwell;
    int mExplosionRadius;
    int mDroppedSkulls;
};

#endif
