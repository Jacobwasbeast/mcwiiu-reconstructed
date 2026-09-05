#include "game/entity/ai/MoveControl.hpp"

#include <math.h>

#include "game/core/AttributeInstance.hpp"
#include "game/core/JumpControl.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

MoveControl::MoveControl(Mob* mob)
    : mMob(mob), mWantedX(0.0), mWantedY(0.0), mWantedZ(0.0), mSpeedModifier(0.0),
      mStrafeForwards(0.0f), mStrafeRight(0.0f), mOperation(WAIT)
{
}

MoveControl::~MoveControl()
{
}

void MoveControl::copyFrom(MoveControl* other)
{
    mWantedX = other->mWantedX;
    mWantedY = other->mWantedY;
    mWantedZ = other->mWantedZ;
    mOperation = other->mOperation;
    mStrafeForwards = other->mStrafeForwards;
    mStrafeRight = other->mStrafeRight;
    mSpeedModifier = other->mSpeedModifier < 1.0 ? 1.0 : other->mSpeedModifier;
}

void MoveControl::wait()
{
    mOperation = WAIT;
}

bool MoveControl::hasWanted()
{
    return mOperation == MOVE_TO;
}

double MoveControl::getSpeedModifier()
{
    return mSpeedModifier;
}

void MoveControl::setSpeedModifier(double speedModifier)
{
    mSpeedModifier = speedModifier;
}

void MoveControl::setWantedPosition(double x, double y, double z, double speedModifier)
{
    mWantedX = x;
    mWantedY = y;
    mWantedZ = z;
    mSpeedModifier = speedModifier;
    mOperation = MOVE_TO;
}

void MoveControl::strafe(float forwards, float right)
{
    mStrafeForwards = forwards;
    mStrafeRight = right;
    mSpeedModifier = 0.25;
    mOperation = STRAFE;
}

float MoveControl::rotlerp(float from, float to, float max)
{
    float delta = Mth::wrapDegrees(to - from);
    if (delta > max)
    {
        delta = max;
    }
    if (delta < -max)
    {
        delta = -max;
    }
    return from + delta;
}

void MoveControl::tick()
{
    bool aquatic = mMob->isInWater();
    float speed = (float)mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue();

    if (mOperation == STRAFE)
    {
        float length = Mth::sqrt(mStrafeForwards * mStrafeForwards + mStrafeRight * mStrafeRight);
        if (length < 1.0f)
        {
            length = 1.0f;
        }
        float scale = (float)(mSpeedModifier * (double)speed / (double)length);
        float sinYaw = Mth::sin(mMob->mYRot * 0.01745329238474369f);
        float cosYaw = Mth::cos(mMob->mYRot * 0.01745329238474369f);
        mMob->mXxa = mStrafeForwards * cosYaw - mStrafeRight * sinYaw;
        mMob->mZza = mStrafeRight * cosYaw + mStrafeForwards * sinYaw;
        mMob->mXxa *= scale;
        mMob->mZza *= scale;
        mOperation = WAIT;
        return;
    }

    if (mOperation == MOVE_TO)
    {
        mOperation = WAIT;
        double dx = mWantedX - mMob->mX;
        double dy = mWantedY - mMob->mY;
        double dz = mWantedZ - mMob->mZ;
        double horizontal = dx * dx + dz * dz;
        double distance = sqrt(horizontal + dy * dy);
        if (distance < 0.0001)
        {
            mMob->setSpeed(0.0f);
            return;
        }

        if (horizontal > 0.01)
        {
            float wantedYRot = (float)(atan2(dz, dx) * 57.2957763671875) - 90.0f;
            mMob->mYRot = rotlerp(mMob->mYRot, wantedYRot, 30.0f);
        }
        mMob->setSpeed((float)((double)speed * mSpeedModifier));

        if (dy > (double)mMob->mMaxUpStep && horizontal > 0.01)
        {
            JumpControl* jumpControl = static_cast<JumpControl*>(mMob->getJumpControl());
            if (jumpControl != NULL)
            {
                jumpControl->jump();
            }
        }
        else if (aquatic)
        {
            mMob->mYya = dy > 0.0 ? 1.0f : -1.0f;
        }
        return;
    }

    if (mOperation == JUMPING)
    {
        mMob->setSpeed((float)((double)speed * mSpeedModifier));
    }
    else
    {
        mMob->setSpeed(0.0f);
    }
}

double MoveControl::getWantedX()
{
    return mWantedX;
}

double MoveControl::getWantedY()
{
    return mWantedY;
}

double MoveControl::getWantedZ()
{
    return mWantedZ;
}

