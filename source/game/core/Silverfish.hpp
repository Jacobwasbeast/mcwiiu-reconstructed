#ifndef GAME_CORE_SILVERFISH_HPP
#define GAME_CORE_SILVERFISH_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/RandomStrollGoal.hpp"
#include "game/entity/mob/Monster.hpp"

class Block;
class BlockPos;
class BlockState;
class DamageSource;
class DataFixerUpper;
class Level;

class Silverfish : public Monster
{
public:
    class SilverfishMergeWithStoneGoal : public RandomStrollGoal
    {
    public:
        SilverfishMergeWithStoneGoal(Silverfish* silverfish);
        virtual ~SilverfishMergeWithStoneGoal();
        virtual bool canUse();
        virtual bool canContinueToUse();
        virtual void start();
        virtual std::wstring getName();

    private:
        int mDirection;
        bool mShouldMerge;
    };

    class SilverfishWakeUpFriendsGoal : public Goal
    {
    public:
        SilverfishWakeUpFriendsGoal(Silverfish* silverfish);
        virtual ~SilverfishWakeUpFriendsGoal();
        void notifyHurt();
        virtual bool canUse();
        virtual void tick();
        virtual std::wstring getName();

    private:
        Silverfish* mSilverfish;
        int mTicksLeft;
    };

    Silverfish(Level* level);
    virtual ~Silverfish();
    static void create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void registerGoals();
    virtual double getRidingHeight();
    virtual double getEyeHeight();
    virtual void registerAttributes();
    virtual bool makeStepSound();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual bool hurt(const DamageSource* source, float damage);
    virtual const SoundEvent* getDeathLoot();
    virtual void tick();
    virtual void setYBodyRot(float yBodyRot);
    virtual double getWalkTargetValue(const BlockPos& pos);
    virtual bool isDarkEnoughToSpawn();
    virtual bool checkSpawnRules();
    virtual EMobType getMobType();
    virtual unsigned long long GetType();

private:
    SilverfishWakeUpFriendsGoal mWakeUpFriendsGoal;
};

#endif
