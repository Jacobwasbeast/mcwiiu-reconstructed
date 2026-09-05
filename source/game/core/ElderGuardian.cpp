#include "game/core/ElderGuardian.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/world/Level.hpp"

ElderGuardian::ElderGuardian(Level* level)
    : Guardian(level, false)
{
    fjDerivedCtorCalls();
    setSize(mBbWidth * 2.35f, mBbHeight * 2.35f);
    setPersistenceRequired(true);
}

ElderGuardian::~ElderGuardian()
{
}

void ElderGuardian::registerAttributes()
{
    Guardian::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.3);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(8.0);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(80.0);
}

void ElderGuardian::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x5D54400100000000ULL);
}

const void* ElderGuardian::getDefaultLootTable()
{
    return &BuiltInLootTables::ELDER_GUARDIAN;
}

unsigned int ElderGuardian::getAttackDuration()
{
    return 60;
}

void ElderGuardian::setGhost()
{
}

const SoundEvent* ElderGuardian::getAmbientSound()
{
    return isInWaterOrBubble() ? SoundEvents::ELDER_GUARDIAN_AMBIENT
                               : SoundEvents::ELDER_GUARDIAN_AMBIENT_LAND;
}

const SoundEvent* ElderGuardian::getHurtSound(const DamageSource*)
{
    return isInWaterOrBubble() ? SoundEvents::ELDER_GUARDIAN_HURT
                               : SoundEvents::ELDER_GUARDIAN_HURT_LAND;
}

const SoundEvent* ElderGuardian::getDeathSound()
{
    return isInWaterOrBubble() ? SoundEvents::ELDER_GUARDIAN_DEATH
                               : SoundEvents::ELDER_GUARDIAN_DEATH_LAND;
}

const SoundEvent* ElderGuardian::getFlopSound()
{
    return SoundEvents::ELDER_GUARDIAN_FLOP;
}

void ElderGuardian::customServerAiStep()
{
    Mob::customServerAiStep();
}

void ElderGuardian::create(Level* level)
{
    new ElderGuardian(level);
}

unsigned long long ElderGuardian::GetType()
{
    return 0x5D54400100000000ULL;
}
