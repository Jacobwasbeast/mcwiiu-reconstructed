#include "game/core/Drowned.hpp"

#include <cmath>
#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/entity/ai/DrownedPathNavigation.hpp"
#include "game/entity/ai/FleeSunGoal.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/MoveTowardsRestrictionGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/RandomStrollGoal.hpp"
#include "game/entity/ai/RangedAttackGoal.hpp"
#include "game/entity/ai/RestrictSunGoal.hpp"
#include "game/entity/ai/SwimMoveControl.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/RemoveBlockGoal.hpp"
#include "game/world/generation/MoveThroughVillageGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

Drowned::Drowned(Level* level)
    : Zombie(level, false), mIsRangedAttacking(false), mMeleeAttackGoal(NULL),
      mRangedAttackGoal(NULL)
{
    setSwimmer(true);
    fjDerivedCtorCalls();
    mMoveControl = new SwimMoveControl(this);
}

Drowned::~Drowned()
{
    delete mMeleeAttackGoal;
    delete mRangedAttackGoal;
}

void Drowned::create(Level* level)
{
    new Drowned(level);
}

void Drowned::performRangedAttack(boost::shared_ptr<LivingEntity> target, float)
{
    double dx = target->mX - mX;
    double dz = target->mZ - mZ;
    double distance = std::sqrt(dx * dx + dz * dz);
    boost::shared_ptr<ThrownTrident> trident(new ThrownTrident(
        mLevel, boost::static_pointer_cast<LivingEntity>(shared_from_this()),
        not_null_ptr<ItemInstance>(new ItemInstance(Items::trident))));
    trident->shoot(dx, target->mY + target->getEyeHeight() * 0.5 - (mY + getEyeHeight()), dz,
                   1.6f, 14.0f);
    mLevel->addEntity(trident);
}

bool Drowned::isAggressive()
{
    return Zombie::isAggressive();
}

void Drowned::setAggressive(bool aggressive)
{
    not_null_ptr<ItemInstance> item = getMainHandItem();
    if (item->getItem() == Items::trident)
    {
        if (aggressive)
            startUsingItem(InteractionHand::eInteractionHand_MainHand);
        else
            stopUsingItem();
    }
    Zombie::setAggressive(aggressive);
}

bool Drowned::breathesWater()
{
    return true;
}

bool Drowned::checkSpawnRules()
{
    if (mLevel->getDifficulty() == Difficulty::PEACEFUL ||
        mLevel->containsAnysolidBlocking(mBoundingBox))
        return false;
    BlockPos pos(mX, mBoundingBox->mMinY, mZ);
    return mLevel->getBrightness(pos) >= 0.25f;
}

bool Drowned::checkSpawnObstruction()
{
    if (!mLevel->isUnobstructed(mBoundingBox, shared_from_this()))
        return false;
    std::vector<AABB*>* boxes = mLevel->getCollisionAABBs(shared_from_this(), mBoundingBox,
                                                            false, false, false);
    return boxes == NULL || boxes->empty();
}

void Drowned::aiStep()
{
    Zombie::aiStep();
    if (mLevel->mIsClientSide)
        return;

    boost::shared_ptr<LivingEntity> target = getTarget();
    if (!target)
    {
        setAggressive(false);
        return;
    }
    setAggressive(true);

    if (!mRangedAttackGoal)
    {
        not_null_ptr<ItemInstance> item = getMainHandItem();
        if (item->getItem() == Items::trident)
            mRangedAttackGoal = new RangedAttackGoal(this, this, 1.0, 20, 60, 10.0f);
    }
    if (!mRangedAttackGoal)
        return;

    double distance = distanceToSqr(boost::static_pointer_cast<Entity>(target));
    if (!mIsRangedAttacking && distance >= 25.0)
    {
        mGoalSelector.removeGoal(mMeleeAttackGoal);
        mGoalSelector.addGoal(3, mRangedAttackGoal, false);
        mIsRangedAttacking = true;
    }
    if (distance <= 9.0)
    {
        mRangedAttackGoal->stop();
        mGoalSelector.removeGoal(mRangedAttackGoal);
        mGoalSelector.addGoal(3, mMeleeAttackGoal, false);
        mIsRangedAttacking = false;
    }
}

void Drowned::registerAttributes()
{
    Zombie::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.23);
}

void Drowned::registerGoals()
{
    mMeleeAttackGoal = new MeleeAttackGoal(this, 1.0, true);
    mGoalSelector.addGoal(2, new RestrictSunGoal(this), true);
    mGoalSelector.addGoal(2, new FleeSunGoal(this, 1.0), true);
    mGoalSelector.addGoal(3, mMeleeAttackGoal, false);
    mGoalSelector.addGoal(5, new MoveTowardsRestrictionGoal(this, 1.0), true);
    mGoalSelector.addGoal(7, new RandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(8, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
    mGoalSelector.addGoal(6, new MoveThroughVillageGoal(this, 1.0, false), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 10, true, false,
                                                                 NULL, false, false), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(AbstractSkeleton), 10,
                                                                 false, false, NULL, false, false), true);
}

const SoundEvent* Drowned::getAmbientSound()
{
    return isInWater() ? SoundEvents::DROWNED_AMBIENT_WATER : SoundEvents::DROWNED_AMBIENT;
}

const SoundEvent* Drowned::getHurtSound(const DamageSource*)
{
    return isInWater() ? SoundEvents::DROWNED_HURT_WATER : SoundEvents::DROWNED_HURT;
}

const SoundEvent* Drowned::getDeathSound()
{
    return isInWater() ? SoundEvents::DROWNED_DEATH_WATER : SoundEvents::DROWNED_DEATH;
}

const SoundEvent* Drowned::getStepSound()
{
    return SoundEvents::DROWNED_STEP;
}

float Drowned::getWaterSlowDown()
{
    return 0.9f;
}

void Drowned::travel(float xxa, float yya, float zza)
{
    if (!isEffectiveAi() || !isInWaterOrBubble())
    {
        LivingEntity::travel(xxa, yya, zza);
        return;
    }
    PIXBeginNamedEvent(0.25f, "Travel in water");
    double oldYd = mYd;
    moveRelative(xxa, yya, zza, 0.02f);
    move(eMoverType_Self, mXd, mYd, mZd, false);
    mXd *= getWaterSlowDown();
    mZd *= getWaterSlowDown();
    if (!mOnGround)
        mYd = mYd * 0.6 - 0.005;
    if (mOnGround && isFree(mXd, mYd + 0.6 - mY, mZd))
        mYd = 0.3;
    PIXEndNamedEvent();
    updateFallFlying();
}

not_null_ptr<ItemInstance> Drowned::getSkull()
{
    return ItemInstance::EMPTY;
}

const void* Drowned::getDefaultLootTable()
{
    return &BuiltInLootTables::DROWNED;
}

const void* Drowned::getDefaultGearTable()
{
    return mRandom->nextFloat() < 0.85f ? &BuiltInLootTables::DROWNED_RANGED_EQUIPMENT
                                        : &BuiltInLootTables::DROWNED_EQUIPMENT;
}

void Drowned::populateDefaultEquipmentSlots(const DifficultyInstance& difficulty)
{
    setLeftHanded(false);
    Mob::populateDefaultEquipmentSlots(difficulty);
}

DrownedPathNavigation* Drowned::createNavigation(Level* level)
{
    return new DrownedPathNavigation(this, level);
}

unsigned long long Drowned::GetType()
{
    return 0x5D55004400000000ULL;
}
