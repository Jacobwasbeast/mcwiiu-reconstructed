#include "game/core/Guardian.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"

#include <cmath>
#include <typeinfo>

#include "game/core/DamageSource.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/MoveTowardsRestrictionGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/RandomStrollGoal.hpp"
#include "game/entity/ai/WaterBoundPathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"

const EntityDataAccessor<int>* Guardian::DATA_ID_ATTACK_TARGET = NULL;

Guardian::Guardian(Level* level, bool createEntityData)
    : Monster(level), mCachedAttackTarget()
{
    mTailAnimation = 0.0f;
    mTailAnimationOld = 0.0f;
    mTailAnimationSpeed = 0.0f;
    mSpikesAnimation = 0.0f;
    mSpikesAnimationOld = 0.0f;
    mAttackTime = 0;
    mTouchedGround = false;
    mRandomStrollGoal = NULL;
    if (createEntityData)
    {
        fjDerivedCtorCalls();
    }
    mExperienceReward = 10;
    setSize(0.85f, 0.85f);
    delete mMoveControl;
    mMoveControl = new GuardianMoveControl(this);
    mTailAnimation = mRandom->nextFloat();
    mTailAnimationOld = mTailAnimation;
}

Guardian::~Guardian()
{
}

void Guardian::staticCtor()
{
    DATA_ID_ATTACK_TARGET = SynchedEntityData::defineId(EntityType::GUARDIAN, EntityType::MONSTER, EntityDataSerializers::INT);
}

void Guardian::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::GUARDIAN);
}

void Guardian::create(Level* level)
{
    new Guardian(level, true);
}

void Guardian::registerGoals()
{
    MoveTowardsRestrictionGoal* restriction = new MoveTowardsRestrictionGoal(this, 1.0);
    mRandomStrollGoal = new RandomStrollGoal(this, 1.0, 80);
    mGoalSelector.addGoal(4, new GuardianAttackGoal(this), true);
    mGoalSelector.addGoal(5, restriction, true);
    mGoalSelector.addGoal(7, mRandomStrollGoal, true);
    mGoalSelector.addGoal(8, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(8, new LookAtPlayerGoal(this, typeid(Mob), 3.0f, 1.0f), true);
    mGoalSelector.addGoal(9, new RandomLookAroundGoal(this), true);
    Mob::registerGoals();
    mTargetSelector.addGoal(1, new NearestAttackableTargetGoal(
        this, typeid(Player), 10, true, false, new GuardianAttackSelector(this), false, false), true);
}

void Guardian::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(6.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(16.0);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(30.0);
}

PathNavigation* Guardian::createNavigation(Level* level)
{
    return new WaterBoundPathNavigation(this, level, true, false);
}

void Guardian::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_ID_ATTACK_TARGET, 0);
}

unsigned int Guardian::getAttackDuration()
{
    return 80;
}

void Guardian::setActiveAttackTarget(int targetId)
{
    if (mEntityData != NULL && DATA_ID_ATTACK_TARGET != NULL)
        mEntityData->set(DATA_ID_ATTACK_TARGET, targetId, true);
}

bool Guardian::hasActiveAttackTarget()
{
    return mEntityData != NULL && DATA_ID_ATTACK_TARGET != NULL &&
           mEntityData->get(DATA_ID_ATTACK_TARGET) != 0;
}

boost::shared_ptr<Entity> Guardian::getActiveAttackTarget()
{
    if (!hasActiveAttackTarget())
    {
        return boost::shared_ptr<Entity>();
    }
    if (!mLevel->mIsClientSide)
    {
        return getTarget();
    }
    if (mCachedAttackTarget != NULL)
    {
        return mCachedAttackTarget;
    }
    boost::shared_ptr<Entity> target = mLevel->getEntity(mEntityData->get(DATA_ID_ATTACK_TARGET));
    if (target != NULL && (target->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
    {
        mCachedAttackTarget = boost::static_pointer_cast<LivingEntity>(target);
    }
    return mCachedAttackTarget;
}

void Guardian::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    LivingEntity::onSyncedDataUpdated(accessor);
    if (accessor == DATA_ID_ATTACK_TARGET)
    {
        mCachedAttackTarget = boost::shared_ptr<LivingEntity>();
        mAttackTime = 0;
    }
}

int Guardian::getAmbientSoundInterval()
{
    return 160;
}

const SoundEvent* Guardian::getAmbientSound()
{
    return isInWaterOrBubble() ? SoundEvents::GUARDIAN_AMBIENT : SoundEvents::GUARDIAN_AMBIENT_LAND;
}

const SoundEvent* Guardian::getHurtSound(const DamageSource*)
{
    return isInWaterOrBubble() ? SoundEvents::GUARDIAN_HURT : SoundEvents::GUARDIAN_HURT_LAND;
}

const SoundEvent* Guardian::getDeathSound()
{
    return isInWaterOrBubble() ? SoundEvents::GUARDIAN_DEATH : SoundEvents::GUARDIAN_DEATH_LAND;
}

const SoundEvent* Guardian::getFlopSound()
{
    return SoundEvents::GUARDIAN_FLOP;
}

bool Guardian::makeStepSound()
{
    return false;
}

double Guardian::getEyeHeight()
{
    return mBbHeight * 0.5f;
}

double Guardian::getWalkTargetValue(const BlockPos& pos)
{
    if (mLevel->getBlockState(pos)->getMaterial() == Material::WATER)
    {
        return (double)(float)(mLevel->getSeaLevel() + 9.5f);
    }
    return Monster::getWalkTargetValue(pos);
}

double Guardian::getAttackAnimationScale(float partialTicks)
{
    return (mAttackTime + partialTicks) / (float)getAttackDuration();
}

void Guardian::aiStep()
{
    if (mLevel->mIsClientSide)
    {
        mTailAnimationOld = mTailAnimation;
        if (!isInWater())
        {
            bool touchedSurface = mYd > 0.0 && mTouchedGround;
            mTailAnimationSpeed = 2.0f;
            if (touchedSurface && !isSilent())
            {
                mLevel->playLocalSound(mX, mY, mZ, getFlopSound(), getSoundSource(),
                                       1.0f, 1.0f, false, 16.0f);
            }
            if (mYd < 0.0)
            {
                BlockPos pos(shared_from_this());
                mTouchedGround = mLevel->isSolidBlockingBlockInLoadedChunk(pos.below(), false);
            }
            else
            {
                mTouchedGround = false;
            }
            mSpikesAnimationOld = mSpikesAnimation;
            mTailAnimation += mTailAnimationSpeed;
        }
        else
        {
            float speed = mTailAnimationSpeed;
            if (!isMoving())
            {
                speed = (0.125f - speed) * 0.2f + speed;
                mTailAnimationSpeed = speed;
                mSpikesAnimationOld = mSpikesAnimation;
                mTailAnimation += speed;
            }
            else if (speed >= 0.5f)
            {
                speed = (0.5f - speed) * 0.1f + speed;
                mTailAnimationSpeed = speed;
                mSpikesAnimationOld = mSpikesAnimation;
                mTailAnimation += speed;
            }
            else
            {
                mTailAnimationSpeed = 4.0f;
                mSpikesAnimationOld = mSpikesAnimation;
                mTailAnimation += 4.0f;
            }
        }

        if (isInWaterOrBubble())
        {
            if (isMoving())
            {
                mSpikesAnimation -= mSpikesAnimation * 0.25f;
            }
            else
            {
                mSpikesAnimation += (1.0f - mSpikesAnimation) * 0.06f;
            }
        }
        else
        {
            mSpikesAnimation = mRandom->nextFloat();
        }

        if (isMoving() && isInWater())
        {
            Vec3* view = getViewVector(0.0f);
            for (int i = 0; i < 2; ++i)
            {
                double rx = mRandom->nextDouble() - 0.5;
                double ry = mRandom->nextDouble();
                double rz = mRandom->nextDouble() - 0.5;
                mLevel->addParticle(PARTICLE_BUBBLE,
                    mX + rx * mBbWidth - view->x * 1.5,
                    mY + ry * mBbHeight - view->y * 1.5,
                    mZ + rz * mBbWidth - view->z * 1.5,
                    0.0, 0.0, 0.0, arrayWithLength<int>());
            }
        }

        if (hasActiveAttackTarget())
        {
            if (mAttackTime < (int)getAttackDuration())
            {
                ++mAttackTime;
            }
            boost::shared_ptr<Entity> target = getActiveAttackTarget();
            if (target != NULL)
            {
                getLookControl()->setLookAt(target, 90.0f, 90.0f);

                double scale = getAttackAnimationScale(0.0f);
                double dx = target->mX - mX;
                double dy = (target->mY + target->mBbHeight * 0.5f) - (mY + getEyeHeight());
                double dz = target->mZ - mZ;
                double distance = sqrt(dx * dx + dy * dy + dz * dz);
                double t = mRandom->nextDouble();
                if (t < distance)
                {
                    double step = 1.7 - scale;
                    double offset = 1.8 - scale;
                    do
                    {
                        t = t + mRandom->nextDouble() * step + offset;
                        mLevel->addParticle(PARTICLE_BUBBLE,
                            (dx / distance) * t + mX,
                            (dy / distance) * t + mY + getEyeHeight(),
                            (dz / distance) * t + mZ,
                            0.0, 0.0, 0.0, arrayWithLength<int>());
                    } while (t < distance);
                }
            }
        }
    }

    if (!isInWaterOrBubble())
    {
        if (mOnGround)
        {
            mYd += 0.5;
            mXd += (double)((mRandom->nextFloat() * 2.0f) - 1.0f) * 0.4;
            mZd += (double)((mRandom->nextFloat() * 2.0f) - 1.0f) * 0.4;
            mHasImpulse = true;
            mOnGround = false;
            mYRot = mRandom->nextFloat() * 360.0f;
        }
    }
    else
    {
        setAirSupply(300);
    }

    Monster::aiStep();
}

double Guardian::getTailAnimation(float partialTicks)
{
    return mTailAnimationOld + (mTailAnimation - mTailAnimationOld) * partialTicks;
}

double Guardian::getSpikesAnimation(float partialTicks)
{
    return mSpikesAnimationOld + (mSpikesAnimation - mSpikesAnimationOld) * partialTicks;
}

ResourceLocation* Guardian::getDefaultLootTable()
{
    return &BuiltInLootTables::GUARDIAN;
}

bool Guardian::isDarkEnoughToSpawn()
{
    return true;
}

bool Guardian::checkSpawnObstruction()
{
    if (!mLevel->isUnobstructed(mBoundingBox, shared_from_this()))
        return false;
    return mLevel->getCollisionAABBs(shared_from_this(), mBoundingBox, false, false, false)->empty();
}

bool Guardian::checkSpawnRules()
{
    if (mRandom->nextInt(20) != 0)
    {
        BlockPos pos(shared_from_this());
        if (mLevel->canSeeSkyFromBelowWater(pos))
        {
            return false;
        }
    }
    return Monster::checkSpawnRules();
}

bool Guardian::hurt(const DamageSource* source, float damage)
{
    if (!isMoving() && !source->isMagic())
    {
        boost::shared_ptr<Entity> entity = source->getDirectEntity();
        if (entity != NULL
            && (entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
        {
            boost::shared_ptr<LivingEntity> attacker =
                boost::dynamic_pointer_cast<LivingEntity>(entity);
            if (!source->isExplosion())
            {
                boost::shared_ptr<EntityDamageSource> thornsSource =
                    DamageSource::CreateThorns(shared_from_this());
                attacker->hurt(thornsSource.get(), 2.0f);
            }
        }
    }
    if (mRandomStrollGoal != NULL)
        mRandomStrollGoal->trigger();
    return Monster::hurt(source, damage);
}

int Guardian::getMaxHeadXRot()
{
    return 180;
}

void Guardian::travel(float xxa, float yya, float zza)
{
    if (!isEffectiveAi() || !isInWater())
    {
        LivingEntity::travel(xxa, yya, zza);
        return;
    }
    moveRelative(xxa, yya, zza, 0.1f);
    move(eMoverType_Self, mXd, mYd, mZd, false);
    mXd *= 0.9;
    mYd *= 0.9;
    mZd *= 0.9;
    if (!hasActiveAttackTarget() && getTarget() == NULL)
    {
        mYd -= 0.005;
    }
}

MobGroupData* Guardian::finalizeMobSpawn(const DifficultyInstance& difficulty,
                                         MobGroupData* data,
                                         const std::wstring& dataString)
{
    return Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));
}

unsigned long long Guardian::GetType()
{
    return EntityType::GUARDIAN;
}

Guardian::GuardianAttackGoal::GuardianAttackGoal(Guardian* guardian)
    : mGuardian(guardian), mAttackTime(0), mElder(false), mUnused0(0), mUnused1(0), mUnused2(0)
{
    setRequiredControlFlags(3);
}

Guardian::GuardianAttackGoal::~GuardianAttackGoal()
{
}

bool Guardian::GuardianAttackGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mGuardian->getTarget();
    return target != NULL && target->isAlive();
}

bool Guardian::GuardianAttackGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mGuardian->getTarget();
    return Goal::canContinueToUse() && target != NULL && (mElder || mGuardian->distanceToSqr(target) > 9.0);
}

void Guardian::GuardianAttackGoal::start()
{
    mAttackTime = -10;
    mGuardian->getNavigation()->stop();
    boost::shared_ptr<LivingEntity> target = mGuardian->getTarget();
    mGuardian->getLookControl()->setLookAt(target, 90.0f, 90.0f);
    mGuardian->mHasImpulse = true;
}

void Guardian::GuardianAttackGoal::stop()
{
    mGuardian->setActiveAttackTarget(0);
    mGuardian->setTarget(boost::shared_ptr<LivingEntity>());
    if (mGuardian->mRandomStrollGoal != NULL)
    {
        static_cast<RandomStrollGoal*>(mGuardian->mRandomStrollGoal)->trigger();
    }
}

void Guardian::GuardianAttackGoal::tick()
{
    boost::shared_ptr<LivingEntity> target = mGuardian->getTarget();
    if (target == NULL)
    {
        return;
    }
    mGuardian->getNavigation()->stop();
    mGuardian->getLookControl()->setLookAt(target, 90.0f, 90.0f);
    if (!mGuardian->canSee(target))
    {
        mGuardian->setTarget(boost::shared_ptr<LivingEntity>());
        return;
    }
    ++mAttackTime;
    if (mAttackTime == 0)
    {
        mGuardian->setActiveAttackTarget(target->getId());
        mGuardian->mLevel->broadcastEntityEvent(mGuardian->shared_from_this(), EntityEvent::GUARDIAN_ATTACK_SOUND, 0);
        return;
    }
    if (mAttackTime >= (int)mGuardian->getAttackDuration())
    {
        float damage = 1.0f;
        if (mGuardian->mLevel->getDifficulty() == Difficulty::PEACEFUL)
        {
            damage = 3.0f;
        }
        if (mElder)
        {
            damage += 2.0f;
        }
        boost::shared_ptr<DamageSource> indirectMagic =
            DamageSource::CreateIndirectMagic(mGuardian->shared_from_this(), target, NULL);
        EntityDamageSource* mobAttack = DamageSource::CreateMobAttack(target);
        target->hurt(indirectMagic.get(), damage);
        double attackDamage = mGuardian->getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue();
        target->hurt(mobAttack, (float)attackDamage);
        mGuardian->setTarget(boost::shared_ptr<LivingEntity>());
    }
}

std::wstring Guardian::GuardianAttackGoal::getName()
{
    return std::wstring(L"GuardianAttack");
}

Guardian::GuardianAttackSelector::GuardianAttackSelector(Guardian* guardian)
    : mGuardian(guardian)
{
}

Guardian::GuardianAttackSelector::~GuardianAttackSelector()
{
}

bool Guardian::GuardianAttackSelector::apply(boost::shared_ptr<Entity> entity) const
{
    unsigned long long type = entity->GetType();
    if ((type & EntityType::PLAYER) == EntityType::PLAYER || type == EntityType::SQUID)
    {
        return entity->distanceToSqr(mGuardian->shared_from_this()) > 9.0;
    }
    return false;
}

Guardian::GuardianMoveControl::GuardianMoveControl(Guardian* guardian)
    : MoveControl(guardian), mGuardian(guardian)
{
}

Guardian::GuardianMoveControl::~GuardianMoveControl()
{
}

void Guardian::GuardianMoveControl::tick()
{
    if (mOperation == MOVE_TO && !mGuardian->getNavigation()->isDone())
    {
        double dx = getWantedX() - mGuardian->mX;
        double dy = getWantedY() - mGuardian->mY;
        double dz = getWantedZ() - mGuardian->mZ;
        double distance = std::sqrt(dz * dz + dx * dx + dy * dy);
        double angle = std::atan2(dz, dx);
        float newYRot = rotlerp(mGuardian->mYRot,
            (float)(angle * 57.29577951308232) - 90.0f, 30.0f);
        mGuardian->setRot(newYRot, mGuardian->mXRot);

        float speed = mGuardian->getSpeed();
        double attributeSpeed = mGuardian->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue();
        mGuardian->setSpeed((float)((getSpeedModifier() * attributeSpeed - speed) * 0.125 + speed));

        int wiggle = mGuardian->mTickCount + mGuardian->getId();
        double wag1 = std::sin((double)wiggle * 0.5) * 0.05;
        float cosYRot = std::cos((double)mGuardian->mYRot * 57.29577951308232);
        float sinYRot = std::sin((double)mGuardian->mYRot * 57.29577951308232);
        mGuardian->mXd += wag1 * cosYRot;
        mGuardian->mZd += wag1 * sinYRot;

        double wag2 = std::sin((double)wiggle * 0.75) * 0.05 * (sinYRot + cosYRot) * 0.25 + mGuardian->mYd;
        mGuardian->mYd = wag2;
        float speed2 = mGuardian->getSpeed();
        mGuardian->mYd = speed2 * (dy / distance) * 0.1 + wag2;

        LookControl* lookControl = mGuardian->getLookControl();
        double lookX = mGuardian->mX + (dx / distance) * 2.0;
        double lookY = mGuardian->getEyeHeight() + mGuardian->mY + (dy / distance) / distance;
        double lookZ = mGuardian->mZ + (dz / distance) * 2.0;
        double wantedX = lookControl->getWantedX();
        double wantedY = lookControl->getWantedY();
        double wantedZ = lookControl->getWantedZ();
        if (!lookControl->isHasWanted())
        {
            wantedX = lookX;
            wantedY = lookY;
            wantedZ = lookZ;
        }
        lookControl->setLookAt(
            (lookX - wantedX) * 0.125 + wantedX,
            (lookY - wantedY) * 0.125 + wantedY,
            (lookZ - wantedZ) * 0.125 + wantedZ,
            10.0f, 40.0f);
        mGuardian->setMoving(true);
        return;
    }
    mGuardian->setSpeed(0.0f);
    mGuardian->setMoving(false);
}
