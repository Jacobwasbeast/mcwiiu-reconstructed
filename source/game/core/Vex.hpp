#ifndef GAME_CORE_VEX_HPP
#define GAME_CORE_VEX_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/TargetGoal.hpp"
#include "game/entity/mob/Monster.hpp"

class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class Level;
class MobGroupData;
class SoundEvent;
template <class T> class EntityDataAccessor;

class Vex : public Monster
{
public:
    class VexChargeAttackGoal : public Goal
    {
    public:
        explicit VexChargeAttackGoal(Vex* vex);
        virtual ~VexChargeAttackGoal();

        virtual bool canUse();
        virtual bool canContinueToUse();
        virtual void start();
        virtual void stop();
        virtual void tick();
        virtual std::wstring getName();

    private:
        Vex* mVex;
    };

    class VexCopyOwnerTargetGoal : public TargetGoal
    {
    public:
        explicit VexCopyOwnerTargetGoal(PathfinderMob* mob);
        virtual ~VexCopyOwnerTargetGoal();

        virtual bool canUse();
        virtual void start();
        virtual std::wstring getName();
    };

    class VexMoveControl : public MoveControl
    {
    public:
        explicit VexMoveControl(Vex* vex);
        virtual ~VexMoveControl();

        virtual void tick();
    };

    class VexRandomMoveGoal : public Goal
    {
    public:
        explicit VexRandomMoveGoal(Vex* vex);
        virtual ~VexRandomMoveGoal();

        virtual bool canUse();
        virtual bool canContinueToUse();
        virtual void tick();
        virtual std::wstring getName();

    private:
        Vex* mVex;
    };

    explicit Vex(Level* level);
    virtual ~Vex();

    static void create(Level* level);
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);

    virtual unsigned long long GetType();
    virtual void defineSynchedData();
    virtual void tick();
    virtual void move(MoverType type, double x, double y, double z, bool pushedByPiston);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual int getLightColor();
    virtual float getBrightness();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void registerGoals();
    virtual const SoundEvent* getAmbientSound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual ResourceLocation* getDefaultGearTable();
    virtual void populateDefaultEquipmentSlots(const DifficultyInstance& difficulty);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty,
                                           MobGroupData* data,
                                           const std::wstring& dataString);
    virtual void registerAttributes();

    void setLimitedLife(int lifeTicks);
    boost::shared_ptr<Mob> getOwner();
    BlockPos* getBoundOrigin();
    void setBoundOrigin(BlockPos* origin);
    bool getVexFlag(int flag);
    void setVexFlag(int flag, bool value);
    bool isCharging();
    void setIsCharging(bool charging);
    void setOwner(const boost::shared_ptr<Mob>& owner);

private:
    static const EntityDataAccessor<unsigned char>* DATA_FLAGS;

    boost::shared_ptr<Mob> mOwner;
    BlockPos* mBoundOrigin;
    bool mLimitedLife;
    int mLifeTicks;
};

#endif
