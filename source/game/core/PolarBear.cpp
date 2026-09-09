#include "game/core/PolarBear.hpp"

#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

static const unsigned long long ENTITY_TYPE_POLAR_BEAR = 0x2d680006ULL;

const EntityDataAccessor<bool>* PolarBear::DATA_STANDING = NULL;

PolarBear::PolarBear(Level* level)
    : Animal(level), mStandingScale(0.0f), mStandingScaleO(0.0f), mWarningSoundTicks(0)
{
    fjDerivedCtorCalls();
    setSize(1.3f, 1.4f);
}

PolarBear::~PolarBear()
{
}

void PolarBear::create(Level* level)
{
    new PolarBear(level);
}

void PolarBear::staticCtor()
{
    DATA_STANDING = SynchedEntityData::defineId<bool>(ENTITY_TYPE_POLAR_BEAR, 0,
                                                       EntityDataSerializers::BOOLEAN);
}

boost::shared_ptr<AgableMob> PolarBear::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    return boost::shared_ptr<AgableMob>(new PolarBear(mLevel));
}

bool PolarBear::isFood(const not_null_ptr<ItemInstance>&)
{
    return false;
}

void PolarBear::registerGoals()
{
    Mob::registerGoals();
    mGoalSelector.addGoal(1, new PolarBearMeleeAttackGoal(this), true);
    mGoalSelector.addGoal(1, new PolarBearPanicGoal(this), true);
    mTargetSelector.addGoal(1, new PolarBearHurtByTargetGoal(this), true);
    mTargetSelector.addGoal(2, new PolarBearAttackPlayersGoal(this), true);
}

void PolarBear::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(30.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(6.0);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(20.0);
}

const SoundEvent* PolarBear::getAmbientSound()
{
    return isBaby() ? SoundEvents::POLARBEAR_BABY_AMBIENT : SoundEvents::POLARBEAR_AMBIENT;
}

const SoundEvent* PolarBear::getHurtSound(const DamageSource*)
{
    return SoundEvents::POLARBEAR_HURT;
}

const SoundEvent* PolarBear::getDeathSound()
{
    return SoundEvents::POLARBEAR_DEATH;
}

void PolarBear::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::POLARBEAR_STEP, 0.15f, 1.0f);
}

void PolarBear::playWarningSound()
{
    if (mWarningSoundTicks < 1)
    {
        playSound(SoundEvents::POLARBEAR_WARNING, 1.0f, 1.0f);
        mWarningSoundTicks = 40;
    }
}

const void* PolarBear::getDefaultLootTable()
{
    return &BuiltInLootTables::POLAR_BEAR;
}

void PolarBear::defineSynchedData()
{
    Animal::defineSynchedData();
    getEntityData()->createDateItem(DATA_STANDING, false);
}

bool PolarBear::isStanding()
{
    return getEntityData()->get(DATA_STANDING);
}

void PolarBear::tick()
{
    Mob::tick();
    if (mLevel->mIsClientSide)
    {
        mStandingScaleO = mStandingScale;
        if (!isStanding()) mStandingScale = Mth::clamp(mStandingScale - 1.0f, 0.0f, 6.0f);
        else mStandingScale = Mth::clamp(mStandingScale + 1.0f, 0.0f, 6.0f);
    }
    if (mWarningSoundTicks > 0) --mWarningSoundTicks;
}

bool PolarBear::doHurtTarget(boost::shared_ptr<Entity> target)
{
    if (!target) return false;
    boost::shared_ptr<LivingEntity> livingTarget = boost::dynamic_pointer_cast<LivingEntity>(target);
    if (!livingTarget) return false;
    boost::shared_ptr<LivingEntity> attacker = boost::static_pointer_cast<LivingEntity>(shared_from_this());
    EntityDamageSource* source = DamageSource::CreateMobAttack(attacker);
    float damage = static_cast<float>(getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue());
    bool hurt = livingTarget->hurt(source, damage);
    delete source;
    if (hurt) doEnchantDamageEffects(boost::static_pointer_cast<LivingEntity>(shared_from_this()), target);
    return hurt;
}

void PolarBear::setStanding(bool standing)
{
    getEntityData()->set(DATA_STANDING, standing, true);
}

double PolarBear::getStandingAnimationScale(float partialTick)
{
    return static_cast<double>((mStandingScale - mStandingScaleO) * partialTick + mStandingScaleO) / 6.0;
}

float PolarBear::getWaterSlowDown()
{
    return 0.98;
}

MobGroupData* PolarBear::finalizeMobSpawn(const DifficultyInstance&, MobGroupData* data, const std::wstring&)
{
    PolarBearGroupData* group = dynamic_cast<PolarBearGroupData*>(data);
    if (group == NULL)
    {
        group = new PolarBearGroupData();
        group->mIsBaby = true;
    }
    else if (group->mIsBaby)
    {
        setAge(-144000);
    }
    return group;
}

bool PolarBear::checkSpawnRules()
{
    int x = static_cast<int>(mX);
    int y = static_cast<int>(mY);
    int z = static_cast<int>(mZ);
    if (mX < static_cast<double>(x)) --x;
    if (mY < static_cast<double>(y)) --y;
    if (mZ < static_cast<double>(z)) --z;
    BlockPos pos(x, y, z);
    const BlockState* state = mLevel->getBlockState(pos);
    if (state == NULL) return false;
    if (mLevel->getRawBrightness(pos, false) <= 8) return false;
    return Animal::checkSpawnRules();
}

unsigned long long PolarBear::GetType()
{
    return ENTITY_TYPE_POLAR_BEAR;
}

PolarBear::PolarBearGroupData::PolarBearGroupData() : mIsBaby(false)
{
}

PolarBear::PolarBearGroupData::~PolarBearGroupData()
{
}

PolarBear::PolarBearAttackPlayersGoal::PolarBearAttackPlayersGoal(PolarBear* polarBear)
    : NearestAttackableTargetGoal(polarBear, typeid(Player), 20, true, true, NULL, false, true)
{
}

PolarBear::PolarBearAttackPlayersGoal::~PolarBearAttackPlayersGoal()
{
}

bool PolarBear::PolarBearAttackPlayersGoal::canUse()
{
    if (mMob->mLevel->getDifficulty() == Difficulty::PEACEFUL || mMob->isBaby()) return false;
    return NearestAttackableTargetGoal::canUse();
}

double PolarBear::PolarBearAttackPlayersGoal::getFollowDistance()
{
    return TargetGoal::getFollowDistance() * 0.5;
}

PolarBear::PolarBearHurtByTargetGoal::PolarBearHurtByTargetGoal(PolarBear* polarBear)
    : HurtByTargetGoal(polarBear, false, arrayWithLength<unsigned long long>())
{
}

PolarBear::PolarBearHurtByTargetGoal::~PolarBearHurtByTargetGoal()
{
}

void PolarBear::PolarBearHurtByTargetGoal::start()
{
    HurtByTargetGoal::start();
    if (mMob->isBaby())
    {
        alertOthers();
        stop();
    }
}

void PolarBear::PolarBearHurtByTargetGoal::alertOther(boost::shared_ptr<PathfinderMob> mob,
                                                       boost::shared_ptr<LivingEntity> target)
{
    if (mob && mob->GetType() == ENTITY_TYPE_POLAR_BEAR && mob->isBaby() == false)
        HurtByTargetGoal::alertOther(mob, target);
}

PolarBear::PolarBearMeleeAttackGoal::PolarBearMeleeAttackGoal(PolarBear* polarBear)
    : MeleeAttackGoal(polarBear, 1.25, true)
{
}

PolarBear::PolarBearMeleeAttackGoal::~PolarBearMeleeAttackGoal()
{
}

bool PolarBear::PolarBearMeleeAttackGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (mMob->mLevel->getDifficulty() == Difficulty::PEACEFUL && target &&
        (target->GetType() & 0x0c800000ULL) == 0x0c800000ULL)
        return false;
    return MeleeAttackGoal::canUse();
}

void PolarBear::PolarBearMeleeAttackGoal::checkAndPerformAttack(boost::shared_ptr<LivingEntity> target,
                                                                 double distance)
{
    double reach = getAttackReachSqr(target);
    if (distance <= reach && mTicksUntilNextAttack < 1)
    {
        mTicksUntilNextAttack = mAttackInterval;
        mMob->doHurtTarget(target);
        static_cast<PolarBear*>(mMob)->setStanding(false);
    }
    else if (distance + distance < reach)
    {
        mTicksUntilNextAttack = mAttackInterval;
        static_cast<PolarBear*>(mMob)->setStanding(false);
    }
    else
    {
        if (mTicksUntilNextAttack < 1) mTicksUntilNextAttack = mAttackInterval;
        if (mTicksUntilNextAttack < 11)
        {
            static_cast<PolarBear*>(mMob)->setStanding(true);
            static_cast<PolarBear*>(mMob)->playWarningSound();
        }
    }
}

void PolarBear::PolarBearMeleeAttackGoal::stop()
{
    static_cast<PolarBear*>(mMob)->setStanding(false);
    MeleeAttackGoal::stop();
}

double PolarBear::PolarBearMeleeAttackGoal::getAttackReachSqr(boost::shared_ptr<LivingEntity> target)
{
    return static_cast<double>(target->mBbWidth) + 4.0;
}

PolarBear::PolarBearPanicGoal::PolarBearPanicGoal(PolarBear* polarBear)
    : PanicGoal(polarBear, 2.0)
{
}

PolarBear::PolarBearPanicGoal::~PolarBearPanicGoal()
{
}

bool PolarBear::PolarBearPanicGoal::canUse()
{
    if (mMob->getTarget() == NULL && !mMob->isOnFire()) return false;
    return PanicGoal::canUse();
}
