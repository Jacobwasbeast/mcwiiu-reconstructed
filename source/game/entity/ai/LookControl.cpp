#include "game/entity/ai/LookControl.hpp"

#include <math.h>

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Mob.hpp"
#include "game/util/Vec3.hpp"

LookControl::LookControl(Mob* mob)
    : mMob(mob), mYMax(0.0f), mXMax(0.0f), mHasWanted(false), mHasWantedRotation(false),
      mWantedRotation(NULL), mWantedX(0.0), mWantedY(0.0), mWantedZ(0.0)
{
}

LookControl::~LookControl()
{
}

void LookControl::setLookAt(boost::shared_ptr<Entity> entity, float yMaxRotationIncrease, float xMaxRotationIncrease)
{
    mWantedX = entity->mX;
    mWantedZ = entity->mZ;
    if ((entity->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
    {
        mWantedY = entity->mY + entity->getEyeHeight();
    }
    else
    {
        mWantedY = (entity->mBoundingBox->mMinY + entity->mBoundingBox->mMaxY) * 0.5;
    }
    mYMax = yMaxRotationIncrease;
    mXMax = xMaxRotationIncrease;
    mHasWanted = true;
}

void LookControl::setLookAt(double x, double y, double z, float yMaxRotationIncrease, float xMaxRotationIncrease)
{
    mWantedX = x;
    mWantedY = y;
    mWantedZ = z;
    mYMax = yMaxRotationIncrease;
    mXMax = xMaxRotationIncrease;
    mHasWanted = true;
}

void LookControl::setLookRotation(const Vec3* rotation, float yMaxRotationIncrease, float xMaxRotationIncrease)
{
    if (mWantedRotation == NULL)
    {
        mWantedRotation = Vec3::newPermanent(rotation);
        mYMax = yMaxRotationIncrease;
        mHasWantedRotation = true;
        mXMax = xMaxRotationIncrease;
    }
}

float LookControl::rotlerp(float from, float to, float maxDelta)
{
    float delta = Mth::wrapDegrees(to - from);
    if (delta > maxDelta)
    {
        delta = maxDelta;
    }
    if (delta < -maxDelta)
    {
        delta = -maxDelta;
    }
    return from + delta;
}

void LookControl::tick()
{
    if (mMob->GetType() == 0x5D540047ULL)
    {
        return;
    }

    if (!mHasWanted)
    {
        if (!mHasWantedRotation)
        {
            mMob->mYRot = rotlerp(mMob->mYRot, mMob->mYRotO, 10.0f);
            mMob->mXRot = rotlerp(mMob->mXRot, mMob->mXRotO, 10.0f);
        }
        else
        {
            mHasWantedRotation = false;
            mMob->mXRot = Mth::clampRotateDegrees(mMob->mXRot, mWantedRotation->x, mXMax);
            mMob->mYRot = Mth::clampRotateDegrees(mMob->mYRot, mWantedRotation->y, mYMax);
            delete mWantedRotation;
            mWantedRotation = NULL;
        }
    }
    else
    {
        mHasWanted = false;
        double dx = mWantedX - mMob->mX;
        double dz = mWantedZ - mMob->mZ;
        double dy = mWantedY - (mMob->mY + mMob->getEyeHeight());
        double horizontal = sqrt(dx * dx + dz * dz);
        float wantedYRot = (float)(atan2(dz, dx) * 57.2957763671875) - 90.0f;
        float wantedXRot = (float)(-(atan2(dy, horizontal) * 57.2957763671875));
        mMob->mXRot = rotlerp(mMob->mXRot, wantedXRot, mXMax);
        mMob->mYRot = rotlerp(mMob->mYRot, wantedYRot, mYMax);
    }

    float delta = Mth::wrapDegrees(mMob->mYRot - mMob->mYRotO);
    if (!mMob->getNavigation()->isDone())
    {
        if (delta < -75.0f)
        {
            mMob->mYRot = mMob->mYRotO - 75.0f;
        }
        if (delta > 75.0f)
        {
            mMob->mYRot = mMob->mYRotO + 75.0f;
        }
    }
}

bool LookControl::isHasWanted()
{
    return mHasWanted;
}

double LookControl::getWantedX()
{
    return mWantedX;
}

double LookControl::getWantedY()
{
    return mWantedY;
}

double LookControl::getWantedZ()
{
    return mWantedZ;
}

float LookControl::getYMax()
{
    return mYMax;
}

float LookControl::getXMax()
{
    return mXMax;
}
