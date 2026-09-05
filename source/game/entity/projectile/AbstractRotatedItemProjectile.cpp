#include "game/entity/projectile/AbstractRotatedItemProjectile.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
#include "game/core/HitResult.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"

AbstractRotatedItemProjectile::AbstractRotatedItemProjectile(Level* level)
    : Entity(level, true), mXTile(0), mYTile(0), mZTile(0), mInTile(0), mLife(0), mInData(0), mShakeTime(0), mInGround(false), mPickup(0), mCrit(false), mOwnerId(0), mNoPhysicsFlag(false), mSoundDelay(0), mOwner(0)
{
    _init();
}

AbstractRotatedItemProjectile::AbstractRotatedItemProjectile(Level* level, double x, double y, double z)
    : Entity(level, true), mXTile(0), mYTile(0), mZTile(0), mInTile(0), mLife(0), mInData(0), mShakeTime(0), mInGround(false), mPickup(0), mCrit(false), mOwnerId(0), mNoPhysicsFlag(false), mSoundDelay(0), mOwner(0)
{
    _init();
    setPos(x, y, z);
}

AbstractRotatedItemProjectile::AbstractRotatedItemProjectile(Level* level, boost::shared_ptr<LivingEntity> owner)
    : Entity(level, true), mXTile(0), mYTile(0), mZTile(0), mInTile(0), mLife(0), mInData(0), mShakeTime(0), mInGround(false), mPickup(0), mCrit(false), mOwnerId(0), mNoPhysicsFlag(false), mSoundDelay(0), mOwner(0)
{
    _init();
    if (owner)
    {
        setPos(owner->mX, owner->mY + owner->getEyeHeight() - 0.1, owner->mZ);
        mOwner = owner->getId();
    }
}

AbstractRotatedItemProjectile::~AbstractRotatedItemProjectile()
{
}

void AbstractRotatedItemProjectile::shoot(double x, double y, double z, float velocity, float inaccuracy)
{
    double length = Mth::sqrt(x * x + y * y + z * z);
    double spread = 0.0075;
    double gx = mRandom->nextGaussian();
    double gy = mRandom->nextGaussian();
    double gz = mRandom->nextGaussian();
    mXd = (gx * spread * inaccuracy + x / length) * velocity;
    mYd = (gy * spread * inaccuracy + y / length) * velocity;
    mZd = (gz * spread * inaccuracy + z / length) * velocity;
    double horizontal = std::sqrt(mXd * mXd + mZd * mZd);
    mYRot = static_cast<float>(std::atan2(mXd, mZd) * 57.2957763671875);
    mYRotO = mYRot;
    mXRot = static_cast<float>(std::atan2(mYd, horizontal) * 57.2957763671875);
    mXRotO = mXRot;
    fjCheckDerivedConstruction();
}

void AbstractRotatedItemProjectile::shootFromRotation(boost::shared_ptr<Entity> entity, float xRot, float yRot, float offset, float velocity, float inaccuracy)
{
    const double radians = 0.01745329238474369;
    float pitch = xRot * radians;
    float yaw = yRot * radians;
    shoot(-std::sin(yaw) * std::cos(pitch), -std::sin(pitch), std::cos(yaw) * std::cos(pitch), velocity, inaccuracy);
    mXd += entity->mXd;
    mZd += entity->mZd;
    if (!entity->mOnGround)
    {
        mYd += entity->mYd;
    }
    fjCheckDerivedConstruction();
}

void AbstractRotatedItemProjectile::setFlag(int flag, bool value)
{
    if (value)
    {
        mNoPhysicsFlag = true;
    }
    else
    {
        mNoPhysicsFlag = false;
    }
    if ((flag & 2) != 0)
    {
        mNoPhysics = value;
    }
}

void AbstractRotatedItemProjectile::setCritArrow(bool critical)
{
    mCrit = critical;
}

bool AbstractRotatedItemProjectile::onHit(const HitResult* result)
{
    if (result == NULL)
    {
        return false;
    }
    if (result->mType == HitResult::BLOCK)
    {
        BlockPos pos = result->getBlockPos();
        mXTile = pos.x;
        mYTile = pos.y;
        mZTile = pos.z;
        mInGround = true;
        mShakeTime = 7;
        setCritArrow(false);
    }
    else
    {
        mInGround = true;
    }
    return true;
}

void AbstractRotatedItemProjectile::staticCtor()
{
}

void AbstractRotatedItemProjectile::_init()
{
    mXTile = 0;
    mYTile = 0;
    mZTile = 0;
    mInTile = 0;
    mLife = 0;
    mInData = 0;
    mShakeTime = 0;
    mInGround = false;
    mPickup = 0;
    mCrit = false;
    mOwnerId = 0;
    mNoPhysicsFlag = false;
    mSoundDelay = 0;
}

void AbstractRotatedItemProjectile::defineSynchedData()
{
}

unsigned int AbstractRotatedItemProjectile::getOnHitSound() const
{
    return 0;
}

void AbstractRotatedItemProjectile::doPostHurtEffects(const boost::shared_ptr<LivingEntity>& entity)
{
}

boost::shared_ptr<Entity> AbstractRotatedItemProjectile::findHitEntity(Vec3* from, Vec3* to)
{
    return boost::shared_ptr<Entity>();
}

void AbstractRotatedItemProjectile::checkDespawn()
{
    if (++mSoundDelay >= 1200)
    {
        remove();
    }
}

bool AbstractRotatedItemProjectile::makeStepSound()
{
    return false;
}

double AbstractRotatedItemProjectile::getWaterInertia()
{
    return 0.6000000238418579;
}

bool AbstractRotatedItemProjectile::canChangeDimensions()
{
    return false;
}

bool AbstractRotatedItemProjectile::shouldRenderAtSqrDistance(double distanceSqr)
{
    double size = mBoundingBox == NULL ? 10.0 : mBoundingBox->getSize() * 10.0;
    return distanceSqr < size * size;
}

bool AbstractRotatedItemProjectile::isNoPhysics()
{
    return mNoPhysics;
}

bool AbstractRotatedItemProjectile::isCritArrow()
{
    return mCrit;
}

void AbstractRotatedItemProjectile::tick()
{
    baseTick();
    if (mShakeTime > 0)
    {
        --mShakeTime;
    }
    if (mInGround)
    {
        checkDespawn();
        return;
    }
    move(eMoverType_Self, mXd, mYd, mZd, false);
    mXd *= getWaterInertia();
    mYd *= getWaterInertia();
    mZd *= getWaterInertia();
    setRot(mYRot, mXRot);
}

void AbstractRotatedItemProjectile::lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport)
{
    mX = x;
    mY = y;
    mZ = z;
    mYRot = yRot;
    mXRot = xRot;
}

void AbstractRotatedItemProjectile::lerpMotion(double xd, double yd, double zd)
{
    mXd = xd;
    mYd = yd;
    mZd = zd;
}

void AbstractRotatedItemProjectile::move(MoverType type, double x, double y, double z, bool pushedByPiston)
{
    Entity::move(type, x, y, z, pushedByPiston);
}

void AbstractRotatedItemProjectile::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"xTile", mXTile);
    tag->putInt(L"yTile", mYTile);
    tag->putInt(L"zTile", mZTile);
    tag->putShort(L"life", mLife);
    tag->putByte(L"inData", mInData);
    tag->putByte(L"shake", mShakeTime);
    tag->putByte(L"inGround", mInGround ? 1 : 0);
    tag->putByte(L"pickup", mPickup);
    tag->putByte(L"crit", mCrit ? 1 : 0);
}

void AbstractRotatedItemProjectile::readAdditionalSaveData(CompoundTag* tag)
{
    mXTile = tag->getInt(L"xTile");
    mYTile = tag->getInt(L"yTile");
    mZTile = tag->getInt(L"zTile");
    mLife = tag->getShort(L"life");
    mInData = tag->getByte(L"inData");
    mShakeTime = tag->getByte(L"shake");
    mInGround = tag->getByte(L"inGround") == 1;
    mPickup = tag->getByte(L"pickup");
    setCritArrow(tag->getBoolean(L"crit"));
}

void AbstractRotatedItemProjectile::playerTouch(boost::shared_ptr<Player> player)
{
    if (player && mPickup != 0 && mInGround)
    {
        remove();
    }
}

bool AbstractRotatedItemProjectile::isAttackable()
{
    return false;
}

float AbstractRotatedItemProjectile::getEyeHeight()
{
    return 0.0;
}

void AbstractRotatedItemProjectile::setNoPhysics(bool noPhysics)
{
    mNoPhysics = noPhysics;
    setFlag(2, noPhysics);
}

bool AbstractRotatedItemProjectile::shouldTick()
{
    return true;
}
