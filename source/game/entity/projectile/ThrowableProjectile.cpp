#include "game/entity/projectile/ThrowableProjectile.hpp"
#include <cmath>
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"

ThrowableProjectile::ThrowableProjectile(Level* level)
    : Entity(level, true), mOwner(), mProjectileLevel(level), mXTile(0), mYTile(0), mZTile(0), mInTile(0), mShake(0), mInGround(0), mOwnerName(), mOwnerId(0), mOwnerData(7), mParticleSpawnPos(NULL)
{
    _throwableInit();
}

ThrowableProjectile::ThrowableProjectile(Level* level, boost::shared_ptr<LivingEntity> owner)
    : Entity(level, true), mOwner(owner), mProjectileLevel(level), mXTile(0), mYTile(0), mZTile(0), mInTile(0), mShake(0), mInGround(0), mOwnerName(), mOwnerId(0), mOwnerData(7), mParticleSpawnPos(NULL)
{
    _throwableInit();
}

ThrowableProjectile::ThrowableProjectile(Level* level, double x, double y, double z)
    : Entity(level, true), mOwner(), mProjectileLevel(level), mXTile(0), mYTile(0), mZTile(0), mInTile(0), mShake(0), mInGround(0), mOwnerName(), mOwnerId(0), mOwnerData(7), mParticleSpawnPos(NULL)
{
    _throwableInit();
    setPos(x, y, z);
}

ThrowableProjectile::~ThrowableProjectile()
{
    delete mParticleSpawnPos;
}

void ThrowableProjectile::shoot(double x, double y, double z, float velocity, float inaccuracy)
{
    double length = std::sqrt(x * x + y * y + z * z);
    double spread = 0.007499999832361937 * inaccuracy;
    x = (x / length + mRandom->nextGaussian() * spread) * velocity;
    y = (y / length + mRandom->nextGaussian() * spread) * velocity;
    z = (z / length + mRandom->nextGaussian() * spread) * velocity;
    mXd = x;
    mYd = y;
    mZd = z;
    double horizontal = std::sqrt(x * x + z * z);
    mYRot = (float)(std::atan2(x, z) * 57.2957763671875);
    mYRotO = mYRot;
    mXRot = (float)(std::atan2(y, horizontal) * 57.2957763671875);
    mXRotO = mXRot;
    mOwnerId = 0;
}

void ThrowableProjectile::shootFromRotation(boost::shared_ptr<Entity> entity, float xRot, float yRot, float offset, float velocity, float inaccuracy)
{
    const float radians = 0.01745329238474369f;
    double x = -std::sin(yRot * radians) * std::cos(xRot * radians);
    double y = -std::sin((xRot + offset) * radians);
    double z = std::cos(yRot * radians) * std::cos(xRot * radians);
    shoot(x, y, z, velocity, inaccuracy);
    mXd += entity->mXd;
    mZd += entity->mZd;
    if (!entity->mOnGround)
        mYd += entity->mYd;
}

void ThrowableProjectile::fjSetParticleSpawnPos(double x, double y, double z)
{
    if (mParticleSpawnPos == NULL)
        mParticleSpawnPos = Vec3::newPermanent(x, y, z);
}

void ThrowableProjectile::addDataWalkers(DataFixerUpper*, unsigned long long)
{
}

void ThrowableProjectile::_throwableInit()
{
    mXTile = -1;
    mYTile = -1;
    mZTile = -1;
    mInTile = 0;
    mShake = 0;
    mInGround = 0;
    mOwnerName = std::wstring(L"");
    mOwnerId = 0;
    mOwnerData = 0;
    mParticleSpawnPos = NULL;
}

void ThrowableProjectile::defineSynchedData()
{
}

bool ThrowableProjectile::shouldRenderAtSqrDistance(double distanceSqr)
{
    double distance = mBoundingBox->getSize() * 4.0 * 64.0;
    return distanceSqr < distance * distance;
}

void ThrowableProjectile::lerpMotion(double xd, double yd, double zd)
{
    mXd = xd;
    mYd = yd;
    mZd = zd;
    if (mXRot == 0.0f && mYRot == 0.0f)
    {
        double horizontal = std::sqrt(xd * xd + zd * zd);
        mYRot = (float)(std::atan2(xd, zd) * 57.2957763671875);
        mYRotO = mYRot;
        mXRot = (float)(std::atan2(yd, horizontal) * 57.2957763671875);
        mXRotO = mXRot;
    }
}

void ThrowableProjectile::tick()
{
    mXOld = mX;
    mYOld = mY;
    mZOld = mZ;
    baseTick();
    if (mShake > 0)
        --mShake;
    if (!mInGround)
    {
        ++mTickCount;
        mX += mXd;
        mY += mYd;
        mZ += mZd;
        float inertia = isInWater() ? 0.8f : 0.99f;
        mXd *= inertia;
        mYd *= inertia;
        mZd *= inertia;
        mYd -= getGravity();
        setPos(mX, mY, mZ);
    }
}

double ThrowableProjectile::getGravity()
{
    return 0.03;
}

void ThrowableProjectile::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"xTile", mXTile);
    tag->putInt(L"yTile", mYTile);
    tag->putInt(L"zTile", mZTile);
    tag->putInt(L"inTile", mInTile);
    tag->putByte(L"shake", mShake);
    tag->putByte(L"inGround", (unsigned char)mInGround);
    tag->putString(L"ownerName", mOwnerName);
}

void ThrowableProjectile::readAdditionalSaveData(CompoundTag* tag)
{
    mXTile = tag->getInt(L"xTile");
    mYTile = tag->getInt(L"yTile");
    mZTile = tag->getInt(L"zTile");
    mInTile = tag->getInt(L"inTile");
    mShake = tag->getByte(L"shake");
    mInGround = tag->getByte(L"inGround");
    mOwnerName = tag->getString(L"ownerName");
}

boost::shared_ptr<LivingEntity> ThrowableProjectile::getOwner()
{
    return mOwner;
}

Vec3* ThrowableProjectile::fjGetParticleSpawnPos() const
{
    return mParticleSpawnPos;
}

bool ThrowableProjectile::shouldTick()
{
    return true;
}
