#include "game/entity/projectile/FishingHook.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

const EntityDataAccessor<int>* FishingHook::DATA_HOOKED_ENTITY = NULL;

FishingHook::FishingHook(Level* level, boost::shared_ptr<Player> owner)
    : Entity(level, true), mInGround(false), mLife(0), mOwner(), mNibble(0),
      mTimeUntilLured(0), mTimeUntilHooked(0), mFishAngle(0.0f), mOpenWater(0),
      mLuck(0), mLureSpeed(0), mHookedIn()
{
    _init();
    init(owner);
    shoot();
}

FishingHook::FishingHook(Level* level, boost::shared_ptr<Player> owner, double x, double y, double z)
    : Entity(level, true), mInGround(false), mLife(0), mOwner(), mNibble(0),
      mTimeUntilLured(0), mTimeUntilHooked(0), mFishAngle(0.0f), mOpenWater(0),
      mLuck(0), mLureSpeed(0), mHookedIn()
{
    _init();
    init(owner);
    setPos(x, y, z);
    mXo = mX;
    mYo = mY;
    mZo = mZ;
}

FishingHook::~FishingHook()
{
}

void FishingHook::bringInHookedEntity()
{
    if (mOwner && mHookedIn)
    {
        mHookedIn->mXd += (mX - mHookedIn->mX) * 0.1;
        mHookedIn->mYd += (mY - mHookedIn->mY) * 0.1;
        mHookedIn->mZd += (mZ - mHookedIn->mZ) * 0.1;
        mHookedIn->mHasImpulse = true;
    }
}

int FishingHook::retrieve()
{
    int result = 0;
    if (mHookedIn)
    {
        bringInHookedEntity();
        result = 3;
    }
    if (mInGround)
    {
        result = 2;
    }
    remove();
    return result;
}

void FishingHook::_init()
{
    mInGround = false;
    mLife = 0;
    mOwner.reset();
    mHookedIn.reset();
    mNibble = 0;
    mTimeUntilLured = 0;
    mTimeUntilHooked = 0;
    mFishAngle = 0.0f;
    mOpenWater = 0;
    mLuck = 0;
    mLureSpeed = 0;
    setPos(0.0, 0.0, 0.0);
}

void FishingHook::init(const boost::shared_ptr<Player>& owner)
{
    mOwner = owner;
    mOpenWater = 1;
}

void FishingHook::shoot()
{
    const float yaw = mYRot * 0.01745329238474369f;
    const float pitch = mXRot * 0.01745329238474369f;
    const float horizontal = Mth::cos(pitch);
    mXd = -Mth::sin(yaw) * horizontal * 0.3;
    mYd = -Mth::sin(pitch) * 0.3;
    mZd = Mth::cos(yaw) * horizontal * 0.3;
    updateRotation();
}

void FishingHook::setLureSpeed(int lureSpeed)
{
    mLureSpeed = lureSpeed;
}

void FishingHook::setLuck(int luck)
{
    mLuck = luck;
}

void FishingHook::staticCtor()
{
    DATA_HOOKED_ENTITY = NULL;
}

void FishingHook::defineSynchedData()
{
    if (mEntityData && DATA_HOOKED_ENTITY)
    {
        mEntityData->createDateItem(DATA_HOOKED_ENTITY, 0);
    }
}

bool FishingHook::canBeHooked(const boost::shared_ptr<Entity>& entity)
{
    return entity && entity.get() != this && entity->isAlive() && entity->isPickable();
}

bool FishingHook::makeStepSound()
{
    return false;
}

bool FishingHook::shouldRenderAtSqrDistance(double distanceSqr)
{
    return distanceSqr < 4096.0;
}

void FishingHook::handleEntityEvent(unsigned char event, int data)
{
    if (event == 31 && mHookedIn)
    {
        bringInHookedEntity();
    }
    Entity::handleEntityEvent(event, data);
}

int FishingHook::shouldStopFishing()
{
    if (!mOwner || mRemoved)
    {
        return 1;
    }
    return 0;
}

void FishingHook::updateRotation()
{
    const double horizontal = Mth::sqrt(mXd * mXd + mZd * mZd);
    const float targetYRot = (float)(std::atan2(mXd, mZd) * 57.2957763671875);
    const float targetXRot = (float)(std::atan2(mYd, horizontal) * 57.2957763671875);
    mYRot = Mth::clampRotateDegrees(mYRot, targetYRot, 180.0f);
    mXRot = Mth::clampRotateDegrees(mXRot, targetXRot, 180.0f);
}

void FishingHook::setHookedEntity()
{
    if (mEntityData && DATA_HOOKED_ENTITY && mHookedIn)
    {
        mEntityData->set(DATA_HOOKED_ENTITY, mHookedIn->getId() + 1, true);
    }
}

void FishingHook::checkCollision()
{
    if (mHookedIn)
    {
        return;
    }
    mX += mXd;
    mY += mYd;
    mZ += mZd;
    setPos(mX, mY, mZ);
}

void FishingHook::catchingFish(const BlockPos&)
{
    if (mTimeUntilLured > 0)
    {
        --mTimeUntilLured;
    }
    if (mTimeUntilLured == 0 && mTimeUntilHooked == 0)
    {
        mTimeUntilHooked = Mth::nextInt(mRandom, 20, 80);
    }
}

void FishingHook::lerpTo(double, double, double, float, float, int, bool)
{
}

void FishingHook::tick()
{
    if (shouldStopFishing())
    {
        remove();
        return;
    }
    Entity::tick();
    if (mHookedIn)
    {
        mHookedIn->mX = mX;
        mHookedIn->mY = mY;
        mHookedIn->mZ = mZ;
    }
    else
    {
        checkCollision();
        updateRotation();
    }
    if (mTimeUntilHooked > 0)
    {
        --mTimeUntilHooked;
    }
}

void FishingHook::addAdditonalSaveData(CompoundTag*)
{
}

void FishingHook::readAdditionalSaveData(CompoundTag*)
{
}

void FishingHook::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    Entity::onSyncedDataUpdated(accessor);
}

boost::shared_ptr<Player> FishingHook::getOwner()
{
    return mOwner;
}

void FishingHook::remove()
{
    Entity::remove();
    mHookedIn.reset();
}

unsigned long long FishingHook::GetType()
{
    return 0x88000006ULL;
}

bool FishingHook::shouldTick()
{
    return true;
}
