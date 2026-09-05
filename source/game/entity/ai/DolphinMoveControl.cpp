#include "game/entity/ai/DolphinMoveControl.hpp"

#include <math.h>

#include "game/core/AttributeInstance.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/Dolphin.hpp"

DolphinMoveControl::DolphinMoveControl(Dolphin* dolphin)
    : MoveControl(dolphin), mMaxYRotChange(85.0f), mMaxXRotChange(10.0f)
{
}

DolphinMoveControl::~DolphinMoveControl()
{
}

void DolphinMoveControl::tick()
{
    if (mMob->isInWater())
    {
        mMob->mXd += 0.005;
    }

    if (mOperation == MOVE_TO)
    {
        mMob->getNavigation()->stop();
        if (!mMob->getNavigation()->isDone())
        {
            mMob->setNoGravity(true);

            double x = mWantedX - mMob->mX;
            double z = mWantedZ - mMob->mZ;
            double y = mWantedY + 0.5 - mMob->mY;
            if (x * x + y * y + z * z < 0.0001)
            {
                mMob->setSpeed(0.0f);
                return;
            }

            float wantedYRot = (float)(atan2(z, x) * 57.2957763671875) - 90.0f;
            mMob->mYRot = rotlerp(mMob->mYRot, wantedYRot, mMaxXRotChange);

            AttributeInstance* movementSpeed = mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
            double speed = mSpeedModifier * movementSpeed->getValue();

            if (mMob->isInWater())
            {
                speed *= 0.02;
                mMob->setSpeed((float)speed);
                double horizontal = Mth::sqrt(x * x + z * z);
                float wantedXRot = Mth::wrapDegrees(-(float)(atan2(y, horizontal) * 57.2957763671875));
                wantedXRot = Mth::clamp(wantedXRot, -mMaxYRotChange, mMaxYRotChange);
                mMob->mXRot = rotlerp(mMob->mXRot, wantedXRot, 5.0f);
                double pitch = (double)mMob->mXRot * 0.01745329238474369;
                mMob->mXd = cos(pitch) * cos((double)mMob->mYRot * 0.01745329238474369) * speed;
                mMob->mYd = y * speed;
                mMob->mZd = cos(pitch) * sin((double)mMob->mYRot * 0.01745329238474369) * speed;
            }
            else
            {
                speed *= 0.1;
                mMob->setSpeed((float)speed);
                mMob->mXd = cos((double)mMob->mYRot * 0.01745329238474369) * speed;
                mMob->mYd = y * speed;
                mMob->mZd = sin((double)mMob->mYRot * 0.01745329238474369) * speed;
            }
            return;
        }
    }

    mMob->setSpeed(0.0f);
    mMob->mXd = 0.0;
    mMob->mYd = 0.0;
    mMob->mZd = 0.0;
    mMob->setNoGravity(false);
    mOperation = WAIT;
}
