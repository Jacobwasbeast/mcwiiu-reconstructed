#include "game/core/Vex.hpp"

#include <cmath>
#include <typeinfo>

#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"

const EntityDataAccessor<unsigned char>* Vex::DATA_FLAGS = NULL;

Vex::Vex(Level* level)
    : Monster(level),
      mOwner(),
      mBoundOrigin(NULL),
      mLimitedLife(false),
      mLifeTicks(0)
{
    fjDerivedCtorCalls();
    mOnGround = true;
    if (mMoveControl != NULL)
        mMoveControl->tick();
    mMoveControl = new VexMoveControl(this);
    setSize(0.4f, 0.8f);
    mExperienceReward = 3;
}

Vex::~Vex()
{
    delete mBoundOrigin;
    delete mMoveControl;
}

void Vex::create(Level* level)
{
    new Vex(level);
}

void Vex::staticCtor()
{
    DATA_FLAGS = SynchedEntityData::defineId<unsigned char>(0x5D540048ULL,
                                                              ENTITY_TYPE_MONSTER,
                                                              EntityDataSerializers::BYTE);
}

void Vex::move(MoverType type, double x, double y, double z, bool pushedByPiston)
{
    Entity::move(type, x, y, z, pushedByPiston);
    checkInsideBlocks();
}

void Vex::tick()
{
    setNoGravity(true);
    mNoPhysics = true;
    Monster::tick();
    mNoPhysics = false;

    if (mLimitedLife && --mLifeTicks <= 0)
    {
        mLifeTicks = 20;
        mDead = true;
    }
}

void Vex::registerGoals()
{
    Mob::registerGoals();

    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(4, new VexChargeAttackGoal(this), true);
    mGoalSelector.addGoal(8, new VexRandomMoveGoal(this), true);
    mGoalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 0.25f), true);
    mGoalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f), true);

    arrayWithLength<unsigned long long> entityTypes(1, true);
    entityTypes.data[0] = 0x5D540048ULL;
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true, entityTypes), true);
    mTargetSelector.addGoal(2, new VexCopyOwnerTargetGoal(this), true);
    mTargetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
}

void Vex::registerAttributes()
{
    Monster::registerAttributes();
}

void Vex::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_FLAGS, static_cast<unsigned char>(0));
}

void Vex::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, 0x5D540048ULL);
}

void Vex::setLimitedLife(int lifeTicks)
{
    mLifeTicks = lifeTicks;
    mLimitedLife = true;
}

void Vex::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);

    if (tag->contains(L"BoundX") && tag->contains(L"BoundY") && tag->contains(L"BoundZ"))
    {
        setBoundOrigin(new BlockPos(tag->getInt(L"BoundX"),
                                    tag->getInt(L"BoundY"),
                                    tag->getInt(L"BoundZ")));
    }

    if (tag->contains(L"LifeTicks"))
    {
        setLimitedLife(tag->getInt(L"LifeTicks"));
    }
}

void Vex::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);

    if (mBoundOrigin != NULL)
    {
        tag->putInt(L"BoundX", mBoundOrigin->x);
        tag->putInt(L"BoundY", mBoundOrigin->y);
        tag->putInt(L"BoundZ", mBoundOrigin->z);
    }

    if (mLimitedLife)
    {
        tag->putInt(L"LifeTicks", mLifeTicks);
    }
}

boost::shared_ptr<Mob> Vex::getOwner()
{
    return mOwner;
}

BlockPos* Vex::getBoundOrigin()
{
    return mBoundOrigin;
}

void Vex::setBoundOrigin(BlockPos* origin)
{
    delete mBoundOrigin;
    mBoundOrigin = origin;
}

bool Vex::getVexFlag(int flag)
{
    return getSharedFlag(flag);
}

void Vex::setVexFlag(int flag, bool value)
{
    setSharedFlag(flag, value);
}

bool Vex::isCharging()
{
    return getVexFlag(1);
}

void Vex::setIsCharging(bool charging)
{
    setVexFlag(1, charging);
}

void Vex::setOwner(const boost::shared_ptr<Mob>& owner)
{
    mOwner = owner;
}

const SoundEvent* Vex::getAmbientSound()
{
    return SoundEvents::VEX_AMBIENT;
}

const SoundEvent* Vex::getDeathSound()
{
    return SoundEvents::VEX_DEATH;
}

const SoundEvent* Vex::getHurtSound(const DamageSource*)
{
    return SoundEvents::VEX_HURT;
}

ResourceLocation* Vex::getDefaultLootTable()
{
    return &BuiltInLootTables::VEX;
}

ResourceLocation* Vex::getDefaultGearTable()
{
    return &BuiltInLootTables::VEX_GEAR;
}

int Vex::getLightColor()
{
    return 0xf000f0;
}

float Vex::getBrightness()
{
    return 1.0f;
}

MobGroupData* Vex::finalizeMobSpawn(const DifficultyInstance& difficulty,
                                    MobGroupData* data,
                                    const std::wstring& dataString)
{
    Mob::populateDefaultEquipmentEnchantments(difficulty);
    return Mob::finalizeMobSpawn(difficulty, data, dataString);
}

void Vex::populateDefaultEquipmentSlots(const DifficultyInstance& difficulty)
{
    Mob::populateDefaultEquipmentSlots(difficulty);
}

unsigned long long Vex::GetType()
{
    return 0x5D540048ULL;
}

Vex::VexChargeAttackGoal::VexChargeAttackGoal(Vex* vex)
    : Goal(), mVex(vex)
{
}

Vex::VexChargeAttackGoal::~VexChargeAttackGoal()
{
}

bool Vex::VexChargeAttackGoal::canUse()
{
    return mVex->getTarget() && !mVex->isCharging();
}

bool Vex::VexChargeAttackGoal::canContinueToUse()
{
    return mVex->getTarget() && mVex->isCharging();
}

void Vex::VexChargeAttackGoal::start()
{
    boost::shared_ptr<LivingEntity> target = mVex->getTarget();
    if (target)
    {
        Vec3* position = target->getEyePosition(1.0f);
        mVex->mMoveControl->setWantedPosition(position->x, position->y, position->z, 1.0);
        mVex->setIsCharging(true);
    }
}

void Vex::VexChargeAttackGoal::stop()
{
    mVex->setIsCharging(false);
}

void Vex::VexChargeAttackGoal::tick()
{
    boost::shared_ptr<LivingEntity> target = mVex->getTarget();
    if (!target)
    {
        mVex->setIsCharging(false);
        return;
    }

    if (mVex->mBoundingBox->intersectsInner(target->mBoundingBox))
    {
        mVex->doHurtTarget(boost::static_pointer_cast<Entity>(target));
        mVex->setTarget(boost::shared_ptr<LivingEntity>());
    }
    else
    {
        Vec3* position = target->getEyePosition(1.0f);
        mVex->mMoveControl->setWantedPosition(position->x, position->y, position->z, 1.0);
    }
}

std::wstring Vex::VexChargeAttackGoal::getName()
{
    return L"Vex_Charging_Attack";
}

Vex::VexCopyOwnerTargetGoal::VexCopyOwnerTargetGoal(PathfinderMob* mob)
    : TargetGoal(mob, false)
{
}

Vex::VexCopyOwnerTargetGoal::~VexCopyOwnerTargetGoal()
{
}

bool Vex::VexCopyOwnerTargetGoal::canUse()
{
    Vex* vex = static_cast<Vex*>(mMob);
    boost::shared_ptr<Mob> owner = vex->getOwner();
    if (!owner)
    {
        return false;
    }

    boost::shared_ptr<LivingEntity> target = owner->getTarget();
    return target && canAttack(target, false);
}

void Vex::VexCopyOwnerTargetGoal::start()
{
    Vex* vex = static_cast<Vex*>(mMob);
    boost::shared_ptr<Mob> owner = vex->getOwner();
    if (owner)
    {
        vex->setTarget(owner->getTarget());
    }
    TargetGoal::start();
}

std::wstring Vex::VexCopyOwnerTargetGoal::getName()
{
    return L"Vex_Copy_Owner_Target";
}

Vex::VexMoveControl::VexMoveControl(Vex* vex)
    : MoveControl(vex)
{
}

Vex::VexMoveControl::~VexMoveControl()
{
}

void Vex::VexMoveControl::tick()
{
    if (mOperation != MOVE_TO)
    {
        return;
    }

    Vex* vex = static_cast<Vex*>(mMob);
    double dx = mWantedX - vex->mX;
    double dy = mWantedY - vex->mY;
    double dz = mWantedZ - vex->mZ;
    double distance = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (distance < vex->mBoundingBox->getSize())
    {
        wait();
        return;
    }

    vex->mXd += dx / distance * 0.05 * mSpeedModifier;
    vex->mYd += dy / distance * 0.05 * mSpeedModifier;
    vex->mZd += dz / distance * 0.05 * mSpeedModifier;
    vex->mYRot = -static_cast<float>(std::atan2(vex->mXd, vex->mZd) * 57.2957763671875);
    vex->mYHeadRot = vex->mYRot;
}

Vex::VexRandomMoveGoal::VexRandomMoveGoal(Vex* vex)
    : Goal(), mVex(vex)
{
    setRequiredControlFlags(1);
}

Vex::VexRandomMoveGoal::~VexRandomMoveGoal()
{
}

bool Vex::VexRandomMoveGoal::canUse()
{
    return !mVex->mMoveControl->hasWanted() && mVex->mRandom->nextInt(7) == 0;
}

bool Vex::VexRandomMoveGoal::canContinueToUse()
{
    return false;
}

void Vex::VexRandomMoveGoal::tick()
{
    double x = mVex->mX + (mVex->mRandom->nextDouble() - 0.5) * 16.0;
    double y = mVex->mY + (mVex->mRandom->nextDouble() - 0.5) * 16.0;
    double z = mVex->mZ + (mVex->mRandom->nextDouble() - 0.5) * 16.0;

    if (mVex->mBoundOrigin)
    {
        x = mVex->mBoundOrigin->x + (mVex->mRandom->nextDouble() - 0.5) * 7.0;
        y = mVex->mBoundOrigin->y + (mVex->mRandom->nextDouble() - 0.5) * 7.0;
        z = mVex->mBoundOrigin->z + (mVex->mRandom->nextDouble() - 0.5) * 7.0;
    }

    mVex->mMoveControl->setWantedPosition(x, y, z, 1.0);
}

std::wstring Vex::VexRandomMoveGoal::getName()
{
    return L"Vex_Random_Move";
}
