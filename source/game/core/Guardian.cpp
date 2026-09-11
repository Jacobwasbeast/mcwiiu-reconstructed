#include "game/core/Guardian.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"

#include <cmath>

#include "game/core/DamageSource.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/entity/ai/Goal.hpp"
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
    setSize(0.85f, 0.85f);
    if (createEntityData)
    {
        fjDerivedCtorCalls();
    }
    mTailAnimation = mRandom->nextFloat();
    mTailAnimationOld = mTailAnimation;
}

Guardian::~Guardian()
{
}

void Guardian::staticCtor()
{
    DATA_ID_ATTACK_TARGET = SynchedEntityData::defineId(0x5D544000ULL, 0, EntityDataSerializers::INT);
}

void Guardian::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x5D544000ULL);
}

void Guardian::create(Level* level)
{
    new Guardian(level, true);
}

void Guardian::registerGoals()
{
    mRandomStrollGoal = new RandomStrollGoal(this, 1.0, 80);
    mGoalSelector.addGoal(7, new GuardianAttackGoal(this), true);
    Mob::registerGoals();
}

void Guardian::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(6.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(16.0);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(30.0);
}

void Guardian::createNavigation(Level* level)
{
    mNavigation = new WaterBoundPathNavigation(this, level, true, false);
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
    return mCachedAttackTarget;
}

void Guardian::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    LivingEntity::onSyncedDataUpdated(accessor);
    if (accessor == DATA_ID_ATTACK_TARGET)
        mCachedAttackTarget = boost::shared_ptr<LivingEntity>();
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
    mTailAnimationOld = mTailAnimation;
    mTailAnimation += (mRandom->nextFloat() - 0.5f) * 0.1f;
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
        return false;
    }
    return Monster::checkSpawnRules();
}

bool Guardian::hurt(const DamageSource* source, float damage)
{
    if (!mLevel->mIsClientSide && !source->isMagic())
    {
        boost::shared_ptr<Entity> entity = source->getEntity();
        if (entity != NULL
            && (entity->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
        {
            if (!source->isExplosion())
            {
                boost::shared_ptr<LivingEntity> attacker =
                    boost::dynamic_pointer_cast<LivingEntity>(entity);
                if (attacker != NULL)
                {
                    boost::shared_ptr<EntityDamageSource> thornsSource =
                        DamageSource::CreateThorns(shared_from_this());
                    attacker->hurt(thornsSource.get(), 2.0f);
                }
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
    return 0x000000005D544000ULL;
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
    mAttackTime = 0;
    mGuardian->setActiveAttackTarget(0);
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
    if (mGuardian->distanceToSqr(target) > 9.0)
    {
        mGuardian->lookAt(target, 90.0f, 90.0f);
    }
    ++mAttackTime;
    if (mAttackTime == 1)
    {
        mGuardian->setActiveAttackTarget(target->getId());
    }
    if (mAttackTime > mGuardian->getAttackDuration())
    {
        mAttackTime = 0;
        mGuardian->setActiveAttackTarget(0);
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
    if (entity == NULL)
    {
        return false;
    }
    unsigned long long type = entity->GetType();
    return (type & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER ||
           (type & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY;
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
    if (!hasWanted())
    {
        wait();
        return;
    }
    double dx = getWantedX() - mGuardian->mX;
    double dy = getWantedY() - mGuardian->mY;
    double dz = getWantedZ() - mGuardian->mZ;
    double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (distance < 1.0)
    {
        wait();
        return;
    }
    mGuardian->setSpeed((float)getSpeedModifier());
    mGuardian->setRot(mGuardian->rotlerp(mGuardian->mYRot,
        (float)(std::atan2(dz, dx) * 57.29577951308232) - 90.0f, 90.0f), mGuardian->mXRot);
}
