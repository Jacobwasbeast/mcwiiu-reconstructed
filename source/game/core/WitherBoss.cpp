#include "game/core/WitherBoss.hpp"

#include <algorithm>
#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/core/BossEvent.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Mth.hpp"
#include "game/core/WitherSkull.hpp"
#include "game/core/WitherTargetHighestDamage.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FlyingPathNavigation.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/LivingEntitySelector.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/ai/WitherRandomAttackPosGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityCategory.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/WitherSkeleton.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/server/ServerBossEvent.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"

const EntityDataAccessor<int>* WitherBoss::DATA_TARGET_A = NULL;
const EntityDataAccessor<int>* WitherBoss::DATA_TARGET_B = NULL;
const EntityDataAccessor<int>* WitherBoss::DATA_TARGET_C = NULL;
const EntityDataAccessor<int>* WitherBoss::DATA_TARGETS[3] = { NULL, NULL, NULL };
const EntityDataAccessor<int>* WitherBoss::DATA_INVULNERABLE_TICKS = NULL;
const EntityDataAccessor<bool>* WitherBoss::DATA_AERIAL_ATTACK = NULL;
const EntityDataAccessor<bool>* WitherBoss::DATA_CHARGE_ATTACKING = NULL;

Predicate<boost::shared_ptr<Entity> >* LIVING_ENTITY_SELECTOR = new LivingEntitySelector();

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
      mMaxShieldHealth(40),
      mLastDamageSource(NULL),
      mPhase(1),
      mShotDelay(0),
      mStunTicks(0),
      mDyingFrames(0),
      mDeathFlapInterval(15),
      mOverlayAlpha(1.0f),
      mSwellAmount(0.0f),
      mOldSwellAmount(0.0f),
      mChargeWindupTicks(0),
      mCharging(false),
      mAttackCooldown(3),
      mFirerate(20),
      mAttackDelay(20),
      mAttackRange(30.0f),
      mSpawningFrames(200),
      mDiving(false),
      mReloadTicks(0),
      mChargeDirection(NULL),
      mWantsToMove(false),
      mChargeTicks(0),
      mTargetTrackingTicks(0),
      mShieldHealth(40),
      mShotCount(0),
      mReloadDuration(40),
      mSkipReload(false),
      mMaxHealth(0),
      mLastHealthInterval(-1),
      mSpawnSpin(10.0f),
      mDestroyBlocksTick(5),
      mHalfHealth(0),
      mHealthInterval(0),
      mIsPathing(false),
      mBossEvent(NULL)
{
    mXRotHeads[0] = 0.0f;
    mXRotHeads[1] = 0.0f;
    mYRotHeads[0] = 0.0f;
    mYRotHeads[1] = 0.0f;
    mXRotOHeads[0] = 0.0f;
    mXRotOHeads[1] = 0.0f;
    mYRotOHeads[0] = 0.0f;
    mYRotOHeads[1] = 0.0f;
    mNextHeadUpdate[0] = 0;
    mNextHeadUpdate[1] = 0;
    mIdleHeadUpdates[0] = 0;
    mIdleHeadUpdates[1] = 0;

    defineSynchedData();
    registerAttributes();
    fjDerivedCtorCalls();

    mBossEvent = new ServerBossEvent(0xB6A67881, L"", BossEvent::PURPLE, BossEvent::PROGRESS);
    mBossEvent->setDarkenScreen(true);

    mFireImmune = true;
    static_cast<GroundPathNavigation*>(getNavigation())->setCanFloat(true);
    mExperienceReward = 50;
    mCategories |= 0x4004;
    setSize(1.0f, 3.5f);
    setAerialAttack(true);
    mMaxHealth = (int)getMaxHealth();
    setHealth(1.0f);
    mBossEvent->setPercent(0.0f);
    mLastHealthInterval = (int)getMaxHealth();
    mHalfHealth = (int)getMaxHealth() / 2;
    mHealthInterval = (int)getMaxHealth() / 6;
    mLastDamageSource = DamageSource::OUT_OF_WORLD->copy();
    mFlyingSpeed = 0.05f;
}

WitherBoss::~WitherBoss()
{
    if (mLastDamageSource != NULL)
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
    return L10N::GetString(0xB6A67881);
}

void WitherBoss::performRangedAttack(boost::shared_ptr<LivingEntity> target, float)
{
    performRangedAttack(0, target);
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
    return block != Blocks::bedrock
        && block != Blocks::end_portal
        && block != Blocks::end_portal_frame
        && block != Blocks::command_block
        && block != Blocks::BARRIER
        && block != Blocks::STRUCTURE_BLOCK
        && block != Blocks::structure_void
        && block != Blocks::piston_extension
        && block != Blocks::end_gateway;
}

void WitherBoss::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x000000005D540047ULL);
}

void WitherBoss::staticCtor()
{
    DATA_TARGET_A = SynchedEntityData::defineId<int>(0x000000005D540047ULL, 0x000000004D540000ULL, EntityDataSerializers::INT);
    DATA_TARGET_B = SynchedEntityData::defineId<int>(0x000000005D540047ULL, 0x000000004D540000ULL, EntityDataSerializers::INT);
    DATA_TARGET_C = SynchedEntityData::defineId<int>(0x000000005D540047ULL, 0x000000004D540000ULL, EntityDataSerializers::INT);
    DATA_TARGETS[0] = DATA_TARGET_A;
    DATA_TARGETS[1] = DATA_TARGET_B;
    DATA_TARGETS[2] = DATA_TARGET_C;
    DATA_INVULNERABLE_TICKS = SynchedEntityData::defineId<int>(0x000000005D540047ULL, 0x000000004D540000ULL, EntityDataSerializers::INT);
    DATA_AERIAL_ATTACK = SynchedEntityData::defineId<bool>(0x000000005D540047ULL, 0x000000004D540000ULL, EntityDataSerializers::BOOLEAN);
    DATA_CHARGE_ATTACKING = SynchedEntityData::defineId<bool>(0x000000005D540047ULL, 0x000000004D540000ULL, EntityDataSerializers::BOOLEAN);
}

void WitherBoss::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_TARGET_A, getId());
    mEntityData->createDateItem(DATA_TARGET_B, getId());
    mEntityData->createDateItem(DATA_TARGET_C, getId());
    mEntityData->createDateItem(DATA_INVULNERABLE_TICKS, 0);
    mEntityData->createDateItem(DATA_AERIAL_ATTACK, false);
    mEntityData->createDateItem(DATA_CHARGE_ATTACKING, false);
}

void WitherBoss::registerAttributes()
{
    Monster::registerAttributes();
    getAttributes()->registerAttribute(SharedMonsterAttributes::FLYING_SPEED);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(600.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.6000000238418579);
    if (mLevel->getDifficulty() == Difficulty::EASY)
    {
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue((float)(getMaxHealth() * 0.5f));
        mMaxShieldHealth /= 2;
    }
    else if (mLevel->getDifficulty() == Difficulty::NORMAL)
    {
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue((float)(getMaxHealth() * 0.75f));
        mMaxShieldHealth = (int)((float)mMaxShieldHealth * 0.75f);
    }
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(70.0);
    getAttribute(SharedMonsterAttributes::ARMOR)->setBaseValue(4.0);
}

void WitherBoss::setAerialAttack(bool aerialAttack)
{
    mEntityData->set(DATA_AERIAL_ATTACK, aerialAttack, true);
}

void WitherBoss::setCustomName(const std::wstring& name, const std::wstring& displayName, bool visible)
{
    Entity::setCustomName(name, displayName, visible);
    mBossEvent->setName(getDisplayName());
}

bool WitherBoss::getAerialAttack() const
{
    return mEntityData->get(DATA_AERIAL_ATTACK);
}

void WitherBoss::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putBoolean(L"AirAttack", getAerialAttack());
    tag->putInt(L"ShieldHealth", mShieldHealth);
    tag->putInt(L"Phase", mPhase);
    tag->putInt(L"SpawningFrames", mSpawningFrames);
    tag->putInt(L"maxHealth", mMaxHealth);
    tag->putInt(L"lastHealthInterval", mLastHealthInterval);
    tag->putInt(L"dyingFrames", mDyingFrames);
    tag->putFloat(L"overlayAlpha", mOverlayAlpha);
    tag->putFloat(L"swellAmount", mSwellAmount);
    tag->putFloat(L"oldSwellAmount", mOldSwellAmount);
    tag->putInt(L"firerate", mFirerate);
}

int WitherBoss::getInvulnerableTicks()
{
    return mEntityData->get(DATA_INVULNERABLE_TICKS);
}

void WitherBoss::setInvulnerableTicks(int ticks)
{
    mEntityData->set(DATA_INVULNERABLE_TICKS, ticks, true);
}

void WitherBoss::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    setInvulnerableTicks(tag->getInt(L"Invul"));
    if (hasCustomName())
        mBossEvent->setName(getDisplayName());

    bool recomputeThresholds = true;
    if (tag->contains(L"Phase"))
    {
        mPhase = tag->getInt(L"Phase");
        recomputeThresholds = false;
    }
    if (tag->contains(L"AirAttack"))
        setAerialAttack(tag->getBoolean(L"AirAttack"));
    if (tag->contains(L"ShieldHealth"))
        mShieldHealth = tag->getInt(L"ShieldHealth");
    if (tag->contains(L"maxHealth"))
        mMaxHealth = tag->getInt(L"maxHealth");
    if (tag->contains(L"lastHealthInterval"))
        mLastHealthInterval = tag->getInt(L"lastHealthInterval");
    if (tag->contains(L"dyingFrames"))
        mDyingFrames = tag->getInt(L"dyingFrames");
    if (tag->contains(L"swellAmount"))
        mSwellAmount = tag->getFloat(L"swellAmount");
    if (tag->contains(L"oldSwellAmount"))
        mOldSwellAmount = tag->getFloat(L"oldSwellAmount");
    if (tag->contains(L"firerate"))
        mFirerate = tag->getInt(L"firerate");
    if (tag->contains(L"SpawningFrames"))
        mSpawningFrames = tag->getInt(L"SpawningFrames");
    else if (getInvulnerableTicks() <= 0)
        mSpawningFrames = 0;
    if (tag->contains(L"overlayAlpha"))
        mOverlayAlpha = tag->getFloat(L"overlayAlpha");
    else
        mOverlayAlpha = mSpawningFrames < 1 ? 0.0f : 1.0f;

    if (recomputeThresholds)
    {
        int maxHealth = (int)getMaxHealth();
        int health = (int)getHealth();
        mLastHealthInterval = health;
        mHalfHealth = maxHealth / 2;
        mHealthInterval = maxHealth / 6;
        if (health <= mHalfHealth)
            mPhase = 0;
    }
}

void WitherBoss::readAdditionContructionTag(CompoundTag* tag)
{
    setAerialAttack(tag->getBoolean(L"AirAttack"));
    mShieldHealth = tag->getInt(L"ShieldHealth");
    mPhase = tag->getInt(L"Phase");
    mSpawningFrames = tag->getInt(L"SpawningFrames");
    mMaxHealth = tag->getInt(L"maxHealth");
    mLastHealthInterval = tag->getInt(L"lastHealthInterval");
    mDyingFrames = tag->getInt(L"dyingFrames");
    mOverlayAlpha = tag->getFloat(L"overlayAlpha");
    mSwellAmount = tag->getFloat(L"swellAmount");
    mOldSwellAmount = tag->getFloat(L"oldSwellAmount");
    mFirerate = tag->getInt(L"firerate");
}

CompoundTag* WitherBoss::getAdditionContructionTag()
{
    CompoundTag* tag = Entity::getAdditionContructionTag();
    if (tag == NULL)
        tag = new CompoundTag();

    tag->putBoolean(L"AirAttack", getAerialAttack());
    tag->putInt(L"ShieldHealth", mShieldHealth);
    tag->putInt(L"Phase", mPhase);
    tag->putInt(L"SpawningFrames", mSpawningFrames);
    tag->putInt(L"maxHealth", mMaxHealth);
    tag->putInt(L"lastHealthInterval", mLastHealthInterval);
    tag->putInt(L"dyingFrames", mDyingFrames);
    tag->putFloat(L"overlayAlpha", mOverlayAlpha);
    tag->putFloat(L"swellAmount", mSwellAmount);
    tag->putFloat(L"oldSwellAmount", mOldSwellAmount);
    tag->putInt(L"firerate", mFirerate);
    return tag;
}

int WitherBoss::getAlternativeTarget(int head)
{
    return mEntityData->get(DATA_TARGETS[head]);
}

bool WitherBoss::isPowered()
{
    return getHealth() <= (float)(getMaxHealth() * 0.5f);
}

void WitherBoss::generateParticles()
{
    if (!mLevel->mIsClientSide)
        return;

    if (getInvulnerableTicks() > 0)
    {
        for (int i = 0; i < 3; ++i)
        {
            mLevel->addParticle(PARTICLE_MOB_SPELL,
                                mX + mRandom->nextGaussian() * 1.0,
                                mY + (double)(float)(mRandom->nextFloat() * 3.3f),
                                mZ + mRandom->nextGaussian() * 1.0,
                                0.7f, 0.7f, 0.9f, arrayWithLength<int>());
        }
    }

    bool powered = isPowered();
    for (int head = 0; head < 3; ++head)
    {
        double headX = getHeadX(head);
        double headY = getHeadY(head);
        double headZ = getHeadZ(head);
        mLevel->addParticle(PARTICLE_SMOKE,
                            headX + mRandom->nextGaussian() * 0.3f,
                            headY + mRandom->nextGaussian() * 0.3f,
                            headZ + mRandom->nextGaussian() * 0.3f,
                            0.0, 0.0, 0.0, arrayWithLength<int>());
        if (powered && mLevel->mRandom->nextInt(4) == 0)
        {
            mLevel->addParticle(PARTICLE_MOB_SPELL,
                                headX + mRandom->nextGaussian() * 0.3f,
                                headY + mRandom->nextGaussian() * 0.3f,
                                headZ + mRandom->nextGaussian() * 0.3f,
                                0.7f, 0.7f, 0.5f, arrayWithLength<int>());
        }
    }
}

bool WitherBoss::IsChargeAttacking()
{
    return mEntityData->get(DATA_CHARGE_ATTACKING);
}

bool WitherBoss::canShoot()
{
    return mReloadTicks == 0 && !mWantsToMove && !mCharging && mChargeWindupTicks == 0 && mShotDelay < 1;
}

void WitherBoss::aiStep()
{
    generateParticles();
    if (mShotDelay > 0)
        --mShotDelay;
    --mStunTicks;
    if (mStunTicks > 0)
        return;

    mYd *= 0.6000000238418579;

    if (mDyingFrames > 0)
    {
        mDeathTime = -1;
        if (mDyingFrames == 200)
            mOverlayAlpha = 0.0f;
        --mDyingFrames;
        if (mDyingFrames == 0)
        {
            if (!mLevel->mIsClientSide)
            {
                mLevel->explode(shared_from_this(), mX, mY + getEyeHeight(), mZ, 7.0f, false,
                                mLevel->getGameRules()->getBoolean(1), false, false, false, -1.0f);
                LivingEntity::die(mLastDamageSource);
                if (!mLevel->mIsClientSide && mLevel->getGameRules()->getBoolean(4))
                {
                    int reward = getExperienceReward(mLastHurtByPlayer);
                    while (reward > 0)
                    {
                        int value = ExperienceOrb::getExperienceValue(reward);
                        reward -= value;
                        mLevel->addEntity(boost::shared_ptr<ExperienceOrb>(
                            new ExperienceOrb(mLevel, mX, mY, mZ, value)));
                    }
                }
            }
            remove();
            return;
        }
        else if (mDyingFrames % mDeathFlapInterval == 0)
        {
            setAerialAttack(!getAerialAttack());
            if (mDeathFlapInterval > 1)
                --mDeathFlapInterval;
        }
        if (mDyingFrames == 120)
            playSound(SoundEvents::WITHER_DEATH, 1.0f, 1.0f);
        else if (mDyingFrames == 60)
            playSound(SoundEvents::WITHER_DEATH, 0.75f, 1.0f);
        setInvulnerableTicks(mDyingFrames);
        mOverlayAlpha += 0.004999999888241291f;
        mOldSwellAmount = mSwellAmount;
        mSwellAmount += 1.0f;
        return;
    }

    if (mSpawningFrames > 0 && (int)getHealth() > 0)
    {
        --mSpawningFrames;
        if (mSpawningFrames == 0)
        {
            if (!mLevel->mIsClientSide)
            {
                setAerialAttack(true);
                mShieldHealth = mMaxShieldHealth;
                mLevel->explode(shared_from_this(), mX, mY + getEyeHeight(), mZ, 7.0f, false,
                                mLevel->getGameRules()->getBoolean(1), false, false, false, -1.0f);
            }
            playSoundToAllPlayers(SoundEvents::WITHER_SPAWN, 1.0f, 1.0f);
        }
        setInvulnerableTicks(mSpawningFrames);
        heal(ceilf(getMaxHealth() / 200.0f));
        float oldYBodyRot = mYBodyRot;
        mSpawnSpin += 0.20000000298023224f;
        float spunYRot = Mth::wrapDegrees((float)(oldYBodyRot - mSpawnSpin));
        mYBodyRot = spunYRot;
        mYHeadRot = spunYRot;
        mYRot = spunYRot;
        if ((oldYBodyRot < 0.0f && spunYRot >= 0.0f) || mSpawningFrames == 199)
            playSound(SoundEvents::WITHER_SPAWN, 1.0f, 1.0f);
        mYRotHeads[0] = Mth::wrapDegrees((float)(mYRotHeads[0] + mSpawnSpin));
        mYRotHeads[1] = Mth::wrapDegrees((float)(mYRotHeads[1] + mSpawnSpin));
        Monster::aiStep();
        return;
    }

    if (!mLevel->mIsClientSide)
    {
        if (getAlternativeTarget(0) != getId())
        {
            boost::shared_ptr<Entity> alternative = mLevel->getEntity(getAlternativeTarget(0));
            if (alternative)
            {
                if (!mIsPathing)
                {
                    if ((alternative->mY > mY && !mDiving)
                        || (!isPowered() && mY < alternative->mY + 5.0))
                    {
                        double yd = mYd < 0.0 ? 0.0 : mYd;
                        mYd = (0.5 - yd) * 0.6 + yd;
                    }
                    else if (mDiving)
                    {
                        mYd -= 1.0;
                    }
                }
                else if (getMoveControl() != NULL)
                {
                    mYd = getMoveControl()->getWantedY() - mY;
                }
            }
        }
    }

    if (mChargeTicks > 0 && mChargeDirection != NULL)
    {
        double chargeX = mChargeDirection->x;
        double chargeZ = mChargeDirection->z;
        --mChargeTicks;
        if (mChargeTicks == 0)
        {
            mChargeDirection->x = 0.0;
            mXd = 0.0;
            mZd = 0.0;
            mShotDelay = 20;
            mCharging = false;
        }
        else
        {
            mZd = chargeZ + chargeZ;
            mXd = chargeX + chargeX;
        }
    }

    boost::shared_ptr<LivingEntity> target = getTarget();
    if (target)
    {
        Vec3* delta = target->getAttachPos(Entity::ENTITY_LOCATION_MOUNT, 0.0f)
                            ->subtract(getAttachPos(Entity::ENTITY_LOCATION_MOUNT, 0.0f));
        mYHeadRot = Mth::clampRotateDegrees(
            mYHeadRot,
            (float)((double)Mth::atan2((int)delta->z, (int)delta->x) * 57.2957763671875 - 90.0),
            10.0f);
        if (IsChargeAttacking())
            mYBodyRot = mYHeadRot;
    }
    Monster::aiStep();

    if (!mLevel->mIsClientSide)
        mXRot = IsChargeAttacking() ? 90.0f : 0.0f;

    mYRotOHeads[0] = mYRotHeads[0];
    mYRotOHeads[1] = mYRotHeads[1];
    mXRotOHeads[0] = mXRotHeads[0];
    mXRotOHeads[1] = mXRotHeads[1];
    for (int head = 0; head < 2; ++head)
    {
        int targetId = getAlternativeTarget(head + 1);
        boost::shared_ptr<Entity> headTarget;
        if (targetId != getId())
            headTarget = mLevel->getEntity(targetId);
        if (!headTarget)
        {
            mYRotHeads[head] = rotlerp(mYRotHeads[head], mYBodyRot, 10.0f);
            mXRotHeads[head] = rotlerp(mXRotHeads[head], mXRot, 40.0f);
        }
        else
        {
            double headX = getHeadX(head + 1);
            double headY = getHeadY(head + 1);
            double headZ = getHeadZ(head + 1);
            double dx = headTarget->mX - headX;
            double dy = headTarget->mY + headTarget->getEyeHeight() - headY;
            double dz = headTarget->mZ - headZ;
            double horizontal = Mth::sqrt(dx * dx + dz * dz);
            float yRot = (float)(atan2(dz, dx) * 57.2957763671875) - 90.0f;
            float xRot = (float)(atan2(dy, horizontal) * 57.2957763671875);
            mXRotHeads[head] = rotlerp(mXRotHeads[head], -xRot, 40.0f);
            mYRotHeads[head] = rotlerp(mYRotHeads[head], yRot, 10.0f);
        }
    }

    if (canShoot() && getTarget() && ++mTargetTrackingTicks > 100)
    {
        mReloadTicks = 1;
        mTargetTrackingTicks = 0;
    }
}

void WitherBoss::makeInvulnerable()
{
    mInvulnerableTime = 200;
    setInvulnerableTicks(200);
}

void WitherBoss::makeStuckInWeb()
{
}

void WitherBoss::startSeenByPlayer(boost::shared_ptr<ServerPlayer> player)
{
    Entity::startSeenByPlayer(player);
    mBossEvent->addPlayer(player, 0);
}

void WitherBoss::stopSeenByPlayer(boost::shared_ptr<ServerPlayer> player)
{
    Entity::stopSeenByPlayer(player);
    mBossEvent->removePlayer(player);
}

std::map<boost::shared_ptr<ServerPlayer>, int>& WitherBoss::getPlayerParty()
{
    return mBossEvent->getPartyPlayers();
}

int WitherBoss::getArmorValue()
{
    return 4;
}

void WitherBoss::changePhase()
{
    --mPhase;
    mMaxHealth -= (int)getMaxHealth() - mHalfHealth;
    setAerialAttack(false);
    mDiving = true;
    mFirerate = 20;
    mWantsToMove = false;
    mReloadDuration <<= 1;
}

bool WitherBoss::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;

    boost::shared_ptr<Entity> directEntity = source->getDirectEntity();
    if (directEntity && directEntity->GetType() == 0x000000008A400002ULL)
        mStunTicks = 80;

    if (source == DamageSource::DROWN
        || (source->getEntity() && source->getEntity()->GetType() == 0x000000005D540047ULL))
        return false;

    if (mSpawningFrames > 0 && source->getEntity()
        && (source->getEntity()->GetType() & 0x000000000D000000ULL) == 0x000000000D000000ULL)
    {
        boost::shared_ptr<Mob> pushed = boost::static_pointer_cast<Mob>(
            mLevel->getEntity(source->getEntity()->getId()));
        pushed->knockback(shared_from_this(), damage, mX - pushed->mX, mY - pushed->mY);
    }

    if ((getInvulnerableTicks() > 0 && source != DamageSource::OUT_OF_WORLD) || mDyingFrames > 0)
        return false;

    if (isPowered() && directEntity
        && ((directEntity->GetType() & 0x000000008A020000ULL) == 0x000000008A020000ULL
            || directEntity->GetType() == 0x000000008A200006ULL))
        return false;

    boost::shared_ptr<Entity> attacker = source->getEntity();
    if (attacker && (attacker->GetType() & 0x000000000C800000ULL) != 0x000000000C800000ULL
        && (attacker->GetType() & 0x000000000C000000ULL) == 0x000000000C000000ULL
        && boost::static_pointer_cast<LivingEntity>(attacker)->getMobType() == MOB_TYPE_UNDEAD)
        return false;

    if (mDestroyBlocksTick < 1)
        mDestroyBlocksTick = 20;
    mIdleHeadUpdates[0] += 3;
    mIdleHeadUpdates[1] += 3;

    bool hurt = Monster::hurt(source, damage);
    if (hurt)
    {
        int health = (int)getHealth();
        if (mLevel->getDifficulty() != Difficulty::EASY
            && health < mLastHealthInterval - mHealthInterval)
        {
            mFirerate /= 2;
            mLastHealthInterval -= mHealthInterval;
        }
        if (health <= mHalfHealth && mPhase > 0)
            changePhase();

        if (attacker)
        {
            bool visible = canSee(attacker);
            double distanceSqr = getPos(1.0f)
                ->subtract(attacker->mX, attacker->mY, attacker->mZ)->lengthSqr();
            if (attacker == getTarget()
                && (!visible || (float)distanceSqr > mAttackRange * mAttackRange))
                mReloadTicks = 1;
        }

        if (mBossEvent != NULL && attacker && attacker->GetType() == 0x000000000C800001ULL)
            mBossEvent->addPlayer(boost::static_pointer_cast<ServerPlayer>(attacker), (int)damage);
    }
    return hurt;
}

int WitherBoss::getLightColor()
{
    return 0xF000F0;
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
    if (head == 0)
    {
        if (target != getId() && getAlternativeTarget(1) == target)
            setAlternativeTarget(1, getId());
        if (target != getId() && getAlternativeTarget(2) == target)
            setAlternativeTarget(2, getId());
    }
    else if (target != getId())
    {
        if (getAlternativeTarget(0) == target)
            return;
        if (getAlternativeTarget(1) == target)
            return;
        if (getAlternativeTarget(2) == target)
            return;
    }
    mEntityData->set(DATA_TARGETS[head], target, true);
}

double WitherBoss::getHeadYRot(int head)
{
    return mYRotHeads[head];
}

double WitherBoss::getHeadXRot(int head)
{
    return mXRotHeads[head];
}

EMobType WitherBoss::getMobType()
{
    return MOB_TYPE_UNDEAD;
}

void WitherBoss::die(const DamageSource* source)
{
    mDyingFrames = 200;
    mLastDamageSource = source->copy();
    mChargeWindupTicks = 0;
    mCharging = false;
    mStunTicks = 0;
    playSound(getDeathSound(), 0.5f, 1.0f);
    mLevel->broadcastEntityEvent(shared_from_this(), 3, 0);
}

void WitherBoss::handleEntityEvent(unsigned char event, int data)
{
    if (event == 3)
    {
        mDyingFrames = 200;
        return;
    }
    Mob::handleEntityEvent(event, data);
}

double WitherBoss::getOverlayAlpha()
{
    return mOverlayAlpha;
}

double WitherBoss::getSwellAmount(float partialTicks) const
{
    return ((mSwellAmount - mOldSwellAmount) * partialTicks + mOldSwellAmount) / 28.0f;
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
    if (getHealth() > 0.0f && !mLevel->mIsClientSide)
        LivingEntity::heal(amount);
}

bool WitherBoss::canFly()
{
    return true;
}

boost::shared_ptr<LivingEntity> WitherBoss::getTarget()
{
    if (mLevel->mIsClientSide && getAlternativeTarget(0) != getId())
    {
        boost::shared_ptr<Entity> alternative = mLevel->getEntity(getAlternativeTarget(0));
        if (alternative
            && (alternative->GetType() & 0x000000000C000000ULL) == 0x000000000C000000ULL)
            return boost::static_pointer_cast<LivingEntity>(alternative);
    }
    return Mob::getTarget();
}

bool WitherBoss::IsSpawning() const
{
    return mSpawningFrames > 0;
}

MobGroupData* WitherBoss::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring& dataString)
{
    MobGroupData* groupData = Mob::finalizeMobSpawn(difficulty, data, dataString);
    setSize(1.0f, 3.5f);
    setAerialAttack(true);
    mSpawningFrames = 200;
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.6000000238418579);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(70.0);
    makeInvulnerable();
    if (mLevel->getDifficulty() == Difficulty::EASY)
    {
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(getMaxHealth() * 0.5);
        mMaxShieldHealth /= 2;
    }
    else if (mLevel->getDifficulty() == Difficulty::NORMAL)
    {
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(getMaxHealth() * 0.75);
        mMaxShieldHealth = (int)((float)mMaxShieldHealth * 0.75f);
    }
    mMaxHealth = (int)getMaxHealth();
    setHealth(1.0f);
    mLastHealthInterval = (int)getMaxHealth();
    mHalfHealth = (int)getMaxHealth() / 2;
    mHealthInterval = (int)getMaxHealth() / 6;
    return groupData;
}

FlyingPathNavigation* WitherBoss::createNavigation(Level* level)
{
    FlyingPathNavigation* navigation = new FlyingPathNavigation(this, level);
    navigation->setCanOpenDoors(false);
    navigation->setCanFloat(true);
    navigation->setCanPassDoors(true);
    return navigation;
}

void WitherBoss::registerGoals()
{
    mGoalSelector.addGoal(0, new WitherDoNothingGoal(this), true);
    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(2, new WitherRandomAttackPosGoal(this, 1.0f, 40, 10, 7), true);
    mGoalSelector.addGoal(5, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(7, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new WitherTargetHighestDamage(this), true);
    mTargetSelector.addGoal(2, new HurtByTargetGoal(this, false, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Mob), 0, false, false,
                                                               LIVING_ENTITY_SELECTOR, true, false), true);
}

const SoundEvent* WitherBoss::getAmbientSound()
{
    return SoundEvents::WITHER_AMBIENT;
}

const SoundEvent* WitherBoss::getHurtSound(const DamageSource*)
{
    return SoundEvents::WITHER_HURT;
}

const SoundEvent* WitherBoss::getDeathSound()
{
    return SoundEvents::WITHER_DEATH;
}

void WitherBoss::destroyBlocks(Level* level, const AABB* box, int expansion)
{
    if (!level->getGameRules()->getBoolean(1))
        return;

    bool destroyed = false;
    for (int x = (int)box->mMinX - expansion; x <= (int)box->mMaxX + expansion; ++x)
    {
        for (int z = (int)box->mMinZ - expansion; z <= (int)box->mMaxZ + expansion; ++z)
        {
            for (int y = (int)box->mMinY - expansion; y <= (int)box->mMaxY + expansion; ++y)
            {
                const BlockPos pos(x, y, z);
                const BlockState* state = level->getBlockState(pos);
                if (state->getMaterial() != Material::AIR && canDestroy(state->getBlock()))
                {
                    destroyed = level->removeBlock(pos) || destroyed;
                    state->getBlock()->spawnResources(level, pos, state, 1);
                }
            }
        }
    }
    if (destroyed)
        playSound(SoundEvents::WITHER_BREAK_BLOCK, 1.0f, 1.0f);
}

void WitherBoss::customServerAiStep()
{
    if (mBossEvent != NULL)
        mBossEvent->setPercent((float)(getHealth() / getMaxHealth()));
    if (mSpawningFrames > 0)
        return;

    if ((mOnGround || isInWater()) && mDiving)
    {
        mLevel->explode(shared_from_this(), mX, mY + getEyeHeight(), mZ, 7.0f, false,
                        mLevel->getGameRules()->getBoolean(1), false, false, false, -1.0f);
        mDiving = false;
        mStunTicks = 80;
        if (mLevel->getDifficulty() != Difficulty::EASY)
        {
            for (int i = 0; i < 3; ++i)
            {
                int error = 0;
                boost::shared_ptr<Mob> spawned = SpawnEggItem::spawnMobAt(
                    mLevel, EntityIO::getKey(0x000000005D540442ULL), mX, mY, mZ, &error, L"");
                if (spawned)
                {
                    boost::shared_ptr<WitherSkeleton> skeleton =
                        boost::static_pointer_cast<WitherSkeleton>(spawned);
                    if (error == 0)
                    {
                        skeleton->setWitherParent(
                            boost::static_pointer_cast<WitherBoss>(shared_from_this()));
                        skeleton->mInvulnerableTime = 60;
                    }
                }
            }
        }
    }

    Mob::customServerAiStep();
    if (mAttackCooldown > 0)
        --mAttackCooldown;

    if (getTarget() && getTarget()->getId() != getAlternativeTarget(0))
        setAlternativeTarget(0, getTarget()->getId());
    else
        setAlternativeTarget(0, getId());

    if (mAttackDelay > 0 && --mAttackDelay == 0)
    {
        if (canShoot() && getTarget())
        {
            if (getNavigation() != NULL)
                getNavigation()->stop();
            performRangedAttack(0, getTarget());
        }
        mAttackDelay = mFirerate;
    }

    for (int head = 1; head < 3; ++head)
    {
        if (mTickCount >= mNextHeadUpdate[head - 1] && canShoot())
        {
            mAttackCooldown = mFirerate;
            mNextHeadUpdate[head - 1] = mTickCount + 10 + mRandom->nextInt(10);
            const Difficulty* difficulty = mLevel->getDifficulty();
            if ((difficulty == Difficulty::NORMAL || difficulty == Difficulty::HARD)
                && ++mIdleHeadUpdates[head - 1] > 15)
            {
                float x = mRandom->nextFloat((float)mX - 10.0f, (float)mX + 10.0f);
                float y = mRandom->nextFloat((float)mY - 5.0f, (float)mY + 5.0f);
                float z = mRandom->nextFloat((float)mZ - 10.0f, (float)mZ + 10.0f);
                if (head == 1)
                    performRangedAttack(head, x, y, z, true);
                mIdleHeadUpdates[head - 1] = 0;
            }

            int headTargetId = getAlternativeTarget(head);
            if (headTargetId == getId())
            {
                std::vector<boost::shared_ptr<Entity> >* candidates =
                    mLevel->getEntitiesOfClass(typeid(Player),
                                               mBoundingBox->grow(mAttackRange, 8.0, mAttackRange));
                while (!candidates->empty())
                {
                    boost::shared_ptr<Player> candidate = boost::static_pointer_cast<Player>(
                        candidates->at(mRandom->nextInt((int)candidates->size())));
                    if (candidate->hasCategory(EntityCategory::eEntityCategory_Undead)
                        && !candidate->mAbilities.mInstabuild
                        && getTarget() != candidate)
                    {
                        setAlternativeTarget(head, candidate->getId());
                        break;
                    }
                    candidates->erase(std::remove(candidates->begin(), candidates->end(), candidate),
                                      candidates->end());
                }
                delete candidates;
            }
            else
            {
                boost::shared_ptr<Entity> headTarget = mLevel->getEntity(headTargetId);
                bool clearTarget = true;
                if (headTarget && headTarget->isAlive()
                    && distanceToSqr(headTarget) <= (double)(mAttackRange * mAttackRange)
                    && canSee(headTarget))
                    clearTarget = false;

                if (clearTarget)
                {
                    setAlternativeTarget(head, getId());
                }
                else if ((headTarget->GetType() & 0x000000000C800000ULL) == 0x000000000C800000ULL
                         && boost::static_pointer_cast<Player>(headTarget)->mAbilities.mInvulnerable)
                {
                    setAlternativeTarget(head, getId());
                }
                else if ((headTarget->GetType() & 0x000000000C000000ULL) == 0x000000000C000000ULL
                         && (difficulty == Difficulty::NORMAL || difficulty == Difficulty::HARD))
                {
                    performRangedAttack(head, boost::static_pointer_cast<LivingEntity>(headTarget));
                    mNextHeadUpdate[head - 1] = mTickCount + 40 + mRandom->nextInt(20);
                    mIdleHeadUpdates[head - 1] = 0;
                }
            }
        }

        if (getTarget())
            setAlternativeTarget(0, getTarget()->getId());
        else
            setAlternativeTarget(0, getId());

        if (mDestroyBlocksTick > 0 && --mDestroyBlocksTick == 0)
            destroyBlocks(mLevel, mBoundingBox, 1);
    }

    if (mReloadTicks > 0 && --mReloadTicks == 0)
    {
        if (!isPowered())
        {
            mWantsToMove = true;
        }
        else
        {
            mChargeWindupTicks = 20;
            if (getTarget())
            {
                delete mChargeDirection;
                mChargeDirection = Vec3::newPermanent(
                    getTarget()->getPos(1.0f)->subtract(getPos(1.0f))->normalize());
            }
        }
    }

    if (mCharging)
    {
        destroyBlocks(mLevel, mBoundingBox, 2);
        std::vector<boost::shared_ptr<Entity> >* hitMobs =
            mLevel->getEntitiesOfClass(typeid(Mob), mBoundingBox->grow(1.0, 4.0, 1.0));
        for (unsigned int i = 0; i < hitMobs->size(); ++i)
        {
            boost::shared_ptr<Mob> hit = boost::static_pointer_cast<Mob>(hitMobs->at(i));
            if (hit != shared_from_this())
            {
                DamageSource* attack = DamageSource::CreateMobAttack(
                    boost::static_pointer_cast<LivingEntity>(shared_from_this()));
                hit->hurt(attack, 15.0f);
                delete attack;
            }
        }
        delete hitMobs;
    }

    if (mChargeWindupTicks > 0 && --mChargeWindupTicks == 0)
    {
        if (getTarget())
        {
            mChargeTicks = 10;
            mCharging = true;
        }
    }

    if (mTickCount % 20 == 0)
    {
        std::vector<boost::shared_ptr<Entity> >* nearby =
            mLevel->getEntitiesOfClass(typeid(Player), mBoundingBox->grow(50.0, 50.0, 50.0));
        delete nearby;
    }

    mEntityData->set(DATA_CHARGE_ATTACKING, mChargeWindupTicks != 0 || mCharging, true);
}

void WitherBoss::dropDeathLoot(bool, int)
{
    boost::shared_ptr<ItemEntity> dropped = spawnAtLocation(Items::netherStar, 1);
    if (dropped)
        dropped->setExtendedLifetime();

    if (!mLevel->mIsClientSide)
    {
        std::vector<boost::shared_ptr<Entity> >* players =
            mLevel->getEntitiesOfClass(typeid(Player), mBoundingBox->grow(50.0, 100.0, 50.0));
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = players->begin();
             it != players->end(); ++it)
        {
            boost::static_pointer_cast<Player>(*it)->awardStat(GenericStats::killWither(),
                                                               GenericStats::param_killWither());
        }
        delete players;
    }
}

void WitherBoss::checkDespawn()
{
    mNoActionTime = 0;
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
    return mX + (double)Mth::cos((float)((mYBodyRot + (float)(head * 180 - 180))
                                         * 0.01745329238474369f)) * 1.3;
}

double WitherBoss::getHeadY(int head)
{
    if (head < 1)
        return mY + 3.0;
    return mY + 2.2;
}

double WitherBoss::getHeadZ(int head)
{
    if (head < 1)
        return mZ;
    return mZ + (double)Mth::sin((float)((mYBodyRot + (float)(head * 180 - 180))
                                         * 0.01745329238474369f)) * 1.3;
}

float WitherBoss::rotlerp(float from, float to, float maxDelta)
{
    float delta = Mth::wrapDegrees((float)(to - from));
    if (delta > maxDelta)
        delta = maxDelta;
    if (delta < -maxDelta)
        delta = -maxDelta;

    float result = (float)(from + delta);
    if (result >= 180.0f)
        result = (float)(result - 360.0f);
    if (result < -180.0f)
        result = (float)(result + 360.0f);
    return result;
}

void WitherBoss::performRangedAttack(int head, boost::shared_ptr<LivingEntity> target)
{
    if (head == 0)
    {
        mTargetTrackingTicks = 0;
        ++mShotCount;
        performRangedAttack(0, target->mX, target->mY + target->getEyeHeight() * 0.5,
                            target->mZ, (mShotCount & 3) == 0);
        if ((mShotCount & 3) == 0)
        {
            mReloadTicks = mReloadDuration;
            if (!getAerialAttack())
            {
                mSkipReload = !mSkipReload;
                if (!mSkipReload)
                {
                    mReloadTicks = 0;
                    mShotDelay = 60;
                }
            }
        }
    }
    else
    {
        performRangedAttack(head, target->mX, target->mY + target->getEyeHeight() * 0.5,
                            target->mZ, false);
    }
}

void WitherBoss::performRangedAttack(int head, double x, double y, double z, bool charged)
{
    mLevel->levelEvent(boost::shared_ptr<Player>(), 1024, BlockPos(mX, mY, mZ), 0);
    double headX = getHeadX(head);
    double headY = getHeadY(head);
    double headZ = getHeadZ(head);
    boost::shared_ptr<WitherSkull> skull(new WitherSkull(
        mLevel, boost::static_pointer_cast<LivingEntity>(shared_from_this()),
        x - headX, y - headY, z - headZ));
    if (charged)
        skull->setDangerous(true);
    skull->mY = headY;
    skull->mX = headX;
    skull->mZ = headZ;
    mLevel->addEntity(skull);
}

float WitherBoss::getMaxHealth()
{
    return LivingEntity::getMaxHealth();
}

float WitherBoss::getHealth()
{
    return LivingEntity::getHealth();
}

int WitherBoss::getDimension()
{
    return mDimension;
}

unsigned long long WitherBoss::GetType()
{
    return 0x000000005D540047ULL;
}
