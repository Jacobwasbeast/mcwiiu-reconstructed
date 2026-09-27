#include "game/core/EyeOfEnderSignal.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/render/ParticleType.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

EyeOfEnderSignal::EyeOfEnderSignal(Level* level, double x, double y, double z)
    : Entity(level, true), mUnknown(0), mTargetX(0.0), mTargetY(0.0), mTargetZ(0.0), mLife(0), mDropItem(false)
{
    _init();
    setSize(0.25f, 0.25f);
    setPos(x, y, z);
}

EyeOfEnderSignal::EyeOfEnderSignal(Level* level)
    : Entity(level, true), mUnknown(0), mTargetX(0.0), mTargetY(0.0), mTargetZ(0.0), mLife(0), mDropItem(false)
{
    _init();
    setSize(0.25f, 0.25f);
}

EyeOfEnderSignal::~EyeOfEnderSignal()
{
}

void EyeOfEnderSignal::_init()
{
    fjDerivedCtorCalls();
    mUnknown = 0;
    mTargetX = 0.0;
    mTargetY = 0.0;
    mTargetZ = 0.0;
    mLife = 0;
    mDropItem = false;
}

void EyeOfEnderSignal::signalTo(const BlockPos& pos)
{
    double dx = static_cast<double>(pos.x) - mX;
    double dz = static_cast<double>(pos.z) - mZ;
    double distance = std::sqrt(dx * dx + dz * dz);
    if (distance <= 12.0)
    {
        mTargetX = static_cast<double>(pos.x);
        mLife = 0;
        mTargetZ = static_cast<double>(pos.z);
        mTargetY = static_cast<double>(pos.y);
    }
    else
    {
        mTargetX = mX + dx / distance * 12.0;
        mLife = 0;
        mTargetZ = mZ + dz / distance * 8.0;
        mTargetY = mY + 8.0;
    }
    mDropItem = mRandom->nextInt(5) > 0;
}

void EyeOfEnderSignal::defineSynchedData()
{
}

bool EyeOfEnderSignal::shouldRenderAtSqrDistance(double distanceSqr)
{
    double distance = mBoundingBox->getSize() * 4.0 * 64.0;
    return distanceSqr < distance * distance;
}

void EyeOfEnderSignal::lerpMotion(double xd, double yd, double zd)
{
    mXd = xd;
    mYd = yd;
    mZd = zd;
    if (mXRot == 0.0f && mYRot == 0.0f)
    {
        double horizontal = std::sqrt(xd * xd + zd * zd);
        mYRot = static_cast<float>(std::atan2(xd, zd) * 57.2957763671875);
        mYRotO = mYRot;
        mXRot = static_cast<float>(std::atan2(yd, horizontal) * 57.2957763671875);
        mXRotO = mXRot;
    }
}

void EyeOfEnderSignal::tick()
{
    mXo = mX;
    mYo = mY;
    mZo = mZ;
    Entity::tick();

    mX += mXd;
    mY += mYd;
    mZ += mZd;

    double horizontal = std::sqrt(mXd * mXd + mZd * mZd);
    mYRot = static_cast<float>(std::atan2(mXd, mZd) * 57.2957763671875);
    mXRot = static_cast<float>(std::atan2(mYd, horizontal) * 57.2957763671875);
    while (mYRotO - mYRot < -180.0f)
    {
        mYRotO -= 360.0f;
    }
    while (mYRotO - mYRot >= 180.0f)
    {
        mYRotO += 360.0f;
    }
    while (mXRotO - mXRot < -180.0f)
    {
        mXRotO -= 360.0f;
    }
    while (mXRotO - mXRot >= 180.0f)
    {
        mXRotO += 360.0f;
    }
    mYRotO += (mYRot - mYRotO) * 0.2f;
    mXRotO += (mXRot - mXRotO) * 0.2f;

    if (mLevel != NULL)
    {
        double dx = mTargetX - mX;
        double dz = mTargetZ - mZ;
        double distance = std::sqrt(dx * dx + dz * dz);
        double speed = (distance - horizontal) * 0.0025 + horizontal;
        if (distance < 1.0)
        {
            speed *= 0.8;
            mYd *= 0.8;
        }
        mXd = dx / distance * speed;
        mZd = dz / distance * speed;
        double gravity = mY < mTargetY ? 0.015 : -1.0;
        mYd += (gravity - mYd) * 0.015;
    }

    lerpMotion(mXd, mYd, mZd);
    setPos(mX, mY, mZ);

    if (mLevel != NULL && mLevel->mIsClientSide)
    {
        for (int i = 0; i < 4; ++i)
        {
            mLevel->addParticle(PARTICLE_PORTAL, mX - mXd * 0.25, mY - mYd * 0.25,
                mZ - mZd * 0.25, mXd, mYd, mZd, arrayWithLength<int>());
        }
        return;
    }

    ++mLife;
    if (mLife > 80)
    {
        remove();
        if (mDropItem)
        {
            spawnAtLocation(Items::enderEye, 1);
        }
        else
        {
            mLevel->levelEvent(LevelEvent::PARTICLES_EYE_OF_ENDER_DEATH, BlockPos(this), 0);
        }
    }
}

void EyeOfEnderSignal::addAdditonalSaveData(CompoundTag*)
{
}

void EyeOfEnderSignal::readAdditionalSaveData(CompoundTag*)
{
}

float EyeOfEnderSignal::getBrightness()
{
    return 1.0;
}

int EyeOfEnderSignal::getLightColor()
{
    return 0xF000F0;
}

bool EyeOfEnderSignal::isAttackable()
{
    return false;
}

void EyeOfEnderSignal::create(Level* level)
{
    new EyeOfEnderSignal(level);
}

unsigned long long EyeOfEnderSignal::GetType()
{
    return EntityType::EYE_OF_ENDER_SIGNAL;
}
