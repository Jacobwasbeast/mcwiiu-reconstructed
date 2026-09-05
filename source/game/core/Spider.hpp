#ifndef GAME_CORE_SPIDER_HPP
#define GAME_CORE_SPIDER_HPP

#include <string>
#include <typeinfo>
#include <boost/shared_ptr.hpp>

#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/mob/Monster.hpp"
#include "game/entity/mob/MobGroupData.hpp"

class Block;
class BlockPos;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class Level;
class MobEffectInstance;
class MobGroupData;
class Random;
class SynchedEntityData;
template <class T> class EntityDataAccessor;

class Spider : public Monster
{
public:
    class SpiderAttackGoal : public MeleeAttackGoal
    {
    public:
        SpiderAttackGoal(Spider* spider);
        virtual ~SpiderAttackGoal();
        virtual bool canContinueToUse();
        virtual double getAttackReachSqr(boost::shared_ptr<LivingEntity> target);
    };

    class SpiderEffectsGroupData : public MobGroupData
    {
    public:
        SpiderEffectsGroupData();
        virtual ~SpiderEffectsGroupData();
        unsigned long long setRandomEffect(Random* random);
        int mEffect;
    };

    class SpiderTargetGoal : public NearestAttackableTargetGoal
    {
    public:
        SpiderTargetGoal(Spider* spider, const std::type_info& targetType);
        virtual ~SpiderTargetGoal();
        virtual bool canUse();
    };

    Spider(Level* level);
    Spider(Level* level, bool callDerivedConstructor);
    virtual ~Spider();

    static void create(Level* level);
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void registerGoals();
    virtual double getRideHeight();
    virtual PathNavigation* createNavigation(Level* level);
    virtual void defineSynchedData();
    virtual void tick();
    virtual void registerAttributes();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual const void* getDefaultLootTable();
    virtual bool onLadder();
    virtual void makeStuckInWeb();
    virtual EMobType getMobType();
    virtual bool canBeAffected(MobEffectInstance* effect);
    virtual bool isClimbing();
    virtual void setClimbing(bool climbing);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& name);
    virtual double getEyeHeight();
    virtual unsigned long long GetType();

private:
    unsigned char mPathfinderMobTail[0x28];
};

#endif
