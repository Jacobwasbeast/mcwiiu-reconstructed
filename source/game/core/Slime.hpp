#ifndef GAME_CORE_SLIME_HPP
#define GAME_CORE_SLIME_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Mob.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/MoveControl.hpp"

class CompoundTag;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class EntityDataAccessor_Base;
class GroundPathNavigation;
class MobGroupData;
class Random;
class SoundEvent;
template <class T> class EntityDataAccessor;

class Slime : public Mob
{
public:
    class SlimeAttackGoal : public Goal
    {
    public:
        explicit SlimeAttackGoal(Slime* slime);
        virtual ~SlimeAttackGoal();
        virtual bool canUse();
        virtual void start();
        virtual bool canContinueToUse();
        virtual void tick();
        virtual std::wstring getName();
    private:
        Slime* mSlime;
        int mGrowTiredTimer;
    };

    class SlimeFloatGoal : public Goal
    {
    public:
        explicit SlimeFloatGoal(Slime* slime);
        virtual ~SlimeFloatGoal();
        virtual bool canUse();
        virtual void tick();
        virtual std::wstring getName();
    private:
        Slime* mSlime;
    };

    class SlimeKeepOnJumpingGoal : public Goal
    {
    public:
        explicit SlimeKeepOnJumpingGoal(Slime* slime);
        virtual ~SlimeKeepOnJumpingGoal();
        virtual bool canUse();
        virtual void tick();
        virtual std::wstring getName();
    private:
        Slime* mSlime;
    };

    class SlimeMoveControl : public MoveControl
    {
    public:
        explicit SlimeMoveControl(Slime* slime);
        virtual ~SlimeMoveControl();
        void setDirection(float yRot, bool jump);
        virtual void setWantedMovement(double speed);
        virtual void tick();
    private:
        float mYRot;
        int mJumpDelay;
        Slime* mSlime;
        bool mWasOnGround;
    };

    class SlimeRandomDirectionGoal : public Goal
    {
    public:
        explicit SlimeRandomDirectionGoal(Slime* slime);
        virtual ~SlimeRandomDirectionGoal();
        virtual bool canUse();
        virtual void tick();
        virtual std::wstring getName();
    private:
        Slime* mSlime;
        float mChosenDegrees;
        int mChosenTicks;
    };

    Slime(Level* level);
    virtual ~Slime();
    virtual unsigned long long GetType();
    virtual void defineSynchedData();
    virtual void tick();
    virtual void remove();
    virtual void playerTouch(boost::shared_ptr<Player> player);
    virtual void push(boost::shared_ptr<Entity> entity);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual double getEyeHeight();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual float getSoundVolume();
    virtual const void* getDefaultLootTable();
    virtual bool checkSpawnRules();
    virtual int getMaxHeadXRot();
    virtual void jumpFromGround();
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& dataString);

    static void staticCtor();
    static void create(Level* level);
    void _init();
    void registerGoals();
    virtual void setSize(int size, bool reset); 
    int getSize();
    static void addDataWalkers(DataFixerUpper* fixer);
    bool isTiny();
    const void* getParticleType();
    void decreaseSquish();
    int getJumpDelay();
    boost::shared_ptr<Slime> createChild();
    void dealDamage(boost::shared_ptr<LivingEntity> entity);
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    bool isDealsDamage();
    int getAttackDamage();
    const SoundEvent* getSquishSound();
    bool doPlayJumpSound();
    const SoundEvent* getJumpSound();
private:
    static const EntityDataAccessor<int>* DATA_ID_SIZE;
public:
    float mSquishO;
    float mSquish;
protected:
    float mTargetSquish;
private:
    bool mWasOnGround;
};

#endif
