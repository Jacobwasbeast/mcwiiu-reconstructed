#ifndef GAME_CORE_ENDERMITE_HPP
#define GAME_CORE_ENDERMITE_HPP

#include "game/entity/mob/Monster.hpp"

class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class Level;
class SoundEvent;

class Endermite : public Monster
{
public:
    Endermite(Level* level);
    virtual ~Endermite();
    virtual unsigned long long GetType();
    virtual double getEyeHeight();
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual bool makeStepSound();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual const void* getDefaultLootTable();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void tick();
    virtual void setYBodyRot(float yBodyRot);
    virtual double getRidingHeight();
    bool isPlayerSpawned();
    void setPlayerSpawned(bool playerSpawned);
    virtual void aiStep();
    virtual bool isDarkEnoughToSpawn();
    virtual bool checkSpawnRules();
    virtual EMobType getMobType();
    static void create(Level* level);

private:
    int mLifetime;
    bool mPlayerSpawned;
};

#endif
