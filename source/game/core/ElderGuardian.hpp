#ifndef GAME_CORE_ELDERGUARDIAN_HPP
#define GAME_CORE_ELDERGUARDIAN_HPP

#include "game/core/Guardian.hpp"

class DamageSource;
class DataFixerUpper;
class Level;
class SoundEvent;

class ElderGuardian : public Guardian
{
public:
    ElderGuardian(Level* level);
    virtual ~ElderGuardian();

    static void addDataWalkers(DataFixerUpper* fixer);
    static void create(Level* level);

    virtual void registerAttributes();
    unsigned int getAttackDuration();
    void setGhost();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getFlopSound();
    virtual void customServerAiStep();
    virtual const void* getDefaultLootTable();
    virtual unsigned long long GetType();
};

#endif
