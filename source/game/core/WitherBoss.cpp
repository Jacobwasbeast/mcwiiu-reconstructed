#include "game/core/WitherBoss.hpp"

#include <cmath>

#include "game/core/DamageSource.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/FlyingPathNavigation.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/server/ServerBossEvent.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"

WitherBoss::WitherDoNothingGoal::WitherDoNothingGoal(WitherBoss* wither)
    : mWither(wither)
{
    setRequiredControlFlags(7);
}

WitherBoss::WitherDoNothingGoal::~WitherDoNothingGoal()
{
}

bool WitherBoss::WitherDoNothingGoal::canUse()
{
    return mWither->getInvulnerableTicks() > 0;
}

std::wstring WitherBoss::WitherDoNothingGoal::getName()
{
    return L"WitherDoNothing";
}

WitherBoss::WitherBoss(Level* level)
    : Monster(level),
      mSwellAmount(0.0f),
      mOldSwellAmount(0.0f),
      mAlternativeTargets(),
      mNextHeadUpdate(),
      mIdleHeadUpdates(),
      mInvulnerableTicks(0),
      mDestroyBlocksTick(0),
      mShotDelay(0),
      mPhase(1),
      mShieldHealth(0),
      mMaxHealth(0),
      mAerialAttack(false),
      mWantsToMove(false),
      mIsPathing(false),
      mPowered(false),
      mSpawning(false),
      mBossEvent(NULL),
      mLastDamageSource(NULL)
{
    mHeadRotations[0] = 0.0f;
    mHeadRotations[1] = 0.0f;
    mHeadRotations[2] = 0.0f;
    mAlternativeTargets[0] = 0;
    mAlternativeTargets[1] = 0;
    mAlternativeTargets[2] = 0;
    mNextHeadUpdate[0] = 0;
    mNextHeadUpdate[1] = 0;
    mNextHeadUpdate[2] = 0;
    mIdleHeadUpdates[0] = 0;
    mIdleHeadUpdates[1] = 0;
    mIdleHeadUpdates[2] = 0;
    mShieldHealth = 0;
    mMaxHealth = (int)getMaxHealth();
    mBossEvent = new ServerBossEvent(0xb6a67881, L"Wither", BossEvent::PINK, BossEvent::PROGRESS);
    mBossEvent->setDarkenScreen(true);
    setSize(0.9f, 3.5f);
    setAerialAttack(true);
    setHealth(getMaxHealth());
    FlyingPathNavigation* navigation = createNavigation(level);
    if (navigation != NULL)
        navigation->setCanFloat(true);
}

WitherBoss::~WitherBoss()
{
    delete mBossEvent;
    delete mLastDamageSource;
}

void WitherBoss::create(Level* level)
{
    new WitherBoss(level);
}

std::wstring WitherBoss::getDisplayName()
{
    if (hasCustomName())
        return getCustomName();
    return L10N::GetString(0xb6a67881);
}

void WitherBoss::performRangedAttack(boost::shared_ptr<LivingEntity> target, float damage)
{
    if (target)
        performRangedAttack(0, target->mX, target->mY, target->mZ, false);
}

bool WitherBoss::isAggressive()
{
    return false;
}

void WitherBoss::setAggressive(bool)
{
}

bool WitherBoss::canDestroy(Block* block)
{
    return block != NULL;
}

void WitherBoss::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer);
}

void WitherBoss::staticCtor()
{
}

void WitherBoss::defineSynchedData()
{
    Mob::defineSynchedData();
}

void WitherBoss::registerAttributes()
{
    Monster::registerAttributes();
}

void WitherBoss::setAerialAttack(bool aerialAttack)
{
    mAerialAttack = aerialAttack;
}

void WitherBoss::setCustomName(const std::wstring& name, const std::wstring& displayName, bool visible)
{
    Entity::setCustomName(name, displayName, visible);
    if (mBossEvent != NULL)
        mBossEvent->setName(getDisplayName());
}

bool WitherBoss::getAerialAttack() const
{
    return mAerialAttack;
}

void WitherBoss::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(L"Invul", getInvulnerableTicks());
    tag->putInt(L"Phase", mPhase);
    tag->putBoolean(L"AirAttack", mAerialAttack);
    tag->putInt(L"ShieldHealth", mShieldHealth);
    tag->putInt(L"maxHealth", mMaxHealth);
    tag->putInt(L"lastHealthInterval", mDestroyBlocksTick);
}

int WitherBoss::getInvulnerableTicks()
{
    return mInvulnerableTicks;
}

void WitherBoss::setInvulnerableTicks(int ticks)
{
    mInvulnerableTicks = ticks;
}

void WitherBoss::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    if (tag->contains(L"Invul"))
        setInvulnerableTicks(tag->getInt(L"Invul"));
    if (tag->contains(L"Phase"))
        mPhase = tag->getInt(L"Phase");
    if (tag->contains(L"AirAttack"))
        setAerialAttack(tag->getBoolean(L"AirAttack"));
    if (tag->contains(L"ShieldHealth"))
        mShieldHealth = tag->getInt(L"ShieldHealth");
    if (tag->contains(L"maxHealth"))
        mMaxHealth = tag->getInt(L"maxHealth");
    if (tag->contains(L"lastHealthInterval"))
        mDestroyBlocksTick = tag->getInt(L"lastHealthInterval");
}

void WitherBoss::readAdditionContructionTag(CompoundTag* tag)
{
    readAdditionalSaveData(tag);
}

CompoundTag* WitherBoss::getAdditionContructionTag()
{
    return NULL;
}

int WitherBoss::getAlternativeTarget(int head)
{
    return head >= 0 && head < 3 ? mAlternativeTargets[head] : 0;
}

bool WitherBoss::isPowered()
{
    return mPowered;
}

void WitherBoss::generateParticles()
{
    if (mLevel == NULL)
        return;
    for (int i = 0; i < 3; ++i)
        mLevel->addParticle(PARTICLE_SMOKE, false, mX, mY + 2.0, mZ, 0.0, 0.0, 0.0, arrayWithLength<int>());
}

bool WitherBoss::IsChargeAttacking()
{
    return mPhase == 2;
}

bool WitherBoss::canShoot()
{
    return mShotDelay <= 0 && !IsChargeAttacking();
}

void WitherBoss::aiStep()
{
    if (mInvulnerableTicks > 0)
        --mInvulnerableTicks;
    if (mShotDelay > 0)
        --mShotDelay;
    Monster::aiStep();
}

bool WitherBoss::makeInvulnerable()
{
    mInvulnerableTicks = 220;
    mShieldHealth = (int)getMaxHealth() / 2;
    mSpawning = true;
    return true;
}

void WitherBoss::makeStuckInWeb()
{
}

void WitherBoss::startSeenByPlayer(boost::shared_ptr<ServerPlayer> player)
{
    if (mBossEvent != NULL)
        mBossEvent->addPlayer(player, 0);
}

void WitherBoss::stopSeenByPlayer(boost::shared_ptr<ServerPlayer> player)
{
    if (mBossEvent != NULL)
        mBossEvent->removePlayer(player);
}

void WitherBoss::getPlayerParty()
{
}

int WitherBoss::getArmorValue()
{
    return 4;
}

void WitherBoss::changePhase()
{
    mPhase = 2;
    mPowered = true;
}

bool WitherBoss::hurt(const DamageSource* source, float damage)
{
    if (mInvulnerableTicks > 0 || source == NULL)
        return false;
    return LivingEntity::hurt(source, damage);
}

int WitherBoss::getLightColor()
{
    return 0xf000f0;
}

bool WitherBoss::isPickable()
{
    return !mRemoved;
}

void WitherBoss::causeFallDamage(float, float)
{
}

void WitherBoss::addEffect(MobEffectInstance*, const boost::shared_ptr<Entity>&)
{
}

void WitherBoss::setAlternativeTarget(int head, int target)
{
    if (head >= 0 && head < 3)
        mAlternativeTargets[head] = target;
}

double WitherBoss::getHeadYRot(int head)
{
    return head < 1 ? mYRot : mHeadRotations[head - 1];
}

double WitherBoss::getHeadXRot(int head)
{
    return head < 1 ? mXRot : 0.0;
}

EMobType WitherBoss::getMobType()
{
    return MOB_TYPE_UNDEAD;
}

void WitherBoss::die(const DamageSource* source)
{
    mLastDamageSource = const_cast<DamageSource*>(source);
    LivingEntity::die(source);
}

void WitherBoss::handleEntityEvent(unsigned char event, int data)
{
    LivingEntity::handleEntityEvent(event, data);
}

double WitherBoss::getOverlayAlpha()
{
    return 1.0;
}

double WitherBoss::getSwellAmount(float partialTicks) const
{
    return (mOldSwellAmount + (mSwellAmount - mOldSwellAmount) * partialTicks) / 28.0f;
}

bool WitherBoss::wantsToMove()
{
    return mWantsToMove;
}

void WitherBoss::setWantsToMove(bool wantsToMove)
{
    mWantsToMove = wantsToMove;
}

void WitherBoss::setIsPathing(bool pathing)
{
    mIsPathing = pathing;
}

void WitherBoss::setShotDelay(int delay)
{
    mShotDelay = delay;
}

void WitherBoss::heal(float amount)
{
    LivingEntity::heal(amount);
}

bool WitherBoss::canFly()
{
    return true;
}

boost::shared_ptr<LivingEntity> WitherBoss::getTarget()
{
    return Mob::getTarget();
}

bool WitherBoss::IsSpawning() const
{
    return mSpawning;
}

MobGroupData* WitherBoss::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& dataString)
{
    MobGroupData* result = Mob::finalizeMobSpawn(difficulty, data, dataString);
    setSize(1.0f, 3.5f);
    setAerialAttack(true);
    mShotDelay = 200;
    mMaxHealth = (int)getMaxHealth();
    setHealth(1.0f);
    mShieldHealth = (int)getMaxHealth();
    return result;
}

FlyingPathNavigation* WitherBoss::createNavigation(Level* level)
{
    return new FlyingPathNavigation(this, level);
}

void WitherBoss::registerGoals()
{
    mGoalSelector.addGoal(0, new WitherDoNothingGoal(this), true);
}

const SoundEvent* WitherBoss::getAmbientSound()
{
    return NULL;
}

const SoundEvent* WitherBoss::getHurtSound(const DamageSource*)
{
    return NULL;
}

const SoundEvent* WitherBoss::getDeathSound()
{
    return NULL;
}

bool WitherBoss::destroyBlocks(Level*, const AABB*, int)
{
    return false;
}

void WitherBoss::customServerAiStep()
{
    if (mBossEvent != NULL)
        mBossEvent->setPercent(getHealth() / getMaxHealth());
}

void WitherBoss::dropDeathLoot(bool, int)
{
}

void WitherBoss::checkDespawn()
{
    Mob::checkDespawn();
}

bool WitherBoss::canRide(boost::shared_ptr<Entity>)
{
    return false;
}

bool WitherBoss::canChangeDimensions()
{
    return false;
}

double WitherBoss::getHeadX(int head)
{
    if (head < 1)
        return mX;
    return mX + Mth::cos(mYRot + (float)(head - 1) * 180.0f) * 1.3;
}

double WitherBoss::getHeadY(int head)
{
    return mY + (head < 1 ? 3.0 : 2.2);
}

double WitherBoss::getHeadZ(int head)
{
    if (head < 1)
        return mZ;
    return mZ + Mth::sin(mYRot + (float)(head - 1) * 180.0f) * 1.3;
}

float WitherBoss::rotlerp(float from, float to, float max)
{
    return Mth::clampRotateDegrees(from, to, max);
}

void WitherBoss::performRangedAttack(int head, boost::shared_ptr<LivingEntity> target)
{
    if (target)
        performRangedAttack(head, target->mX, target->mY, target->mZ, false);
}

void WitherBoss::performRangedAttack(int, double, double, double, bool)
{
    mShotDelay = 40;
}

float WitherBoss::getMaxHealth()
{
    return 300.0f;
}

float WitherBoss::getHealth()
{
    return LivingEntity::getHealth();
}

unsigned int WitherBoss::getDimension()
{
    return 0;
}

unsigned long long WitherBoss::GetType()
{
    return 0x5d540047ULL;
}
