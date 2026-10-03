#include "game/entity/ai/DolphinMoveControl.hpp"

#include <math.h>

#include "game/core/AttributeInstance.hpp"
#include "game/core/Dolphin.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"

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
        mMob->mYd += 0.005;
    }

    if (mOperation == MOVE_TO && !mMob->getNavigation()->isDone())
    {
        mMob->setMoving(true);

        double dx = mWantedX - mMob->mX;
        double dz = mWantedZ - mMob->mZ;
        double dy = mWantedY + 0.5 - mMob->mY;
        if (dy * dy + dx * dx + dz * dz < 0.0001)
        {
            mMob->setZza(0.0f);
            return;
        }

        float wantedYRot = (float)(atan2(dz, dx) * 57.2957763671875) - 90.0f;
        mMob->mYRot = rotlerp(mMob->mYRot, wantedYRot, mMaxXRotChange);
        mMob->mYBodyRot = mMob->mYRot;
        mMob->mYHeadRot = mMob->mYRot;

        AttributeInstance* movementSpeed = mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
        float speed = (float)(mSpeedModifier * movementSpeed->getValue());

        if (mMob->isInWater())
        {
            mMob->setSpeed(speed * 0.02f);

            double horizontal = Mth::sqrt(dx * dx + dz * dz);
            float wantedXRot = Mth::wrapDegrees(-(float)(atan2(dy, horizontal) * 57.2957763671875));
            wantedXRot = Mth::clamp(wantedXRot, -mMaxYRotChange, mMaxYRotChange);
            mMob->mXRot = rotlerp(mMob->mXRot, wantedXRot, 5.0f);
            mMob->setZza(Mth::cos(mMob->mXRot * 0.017453292f) * speed);
            mMob->setYya((float)dy * speed);
        }
        else
        {
            mMob->setSpeed(speed * 0.1f);
        }

        return;
    }

    mMob->setSpeed(0.0f);
    mMob->setXxa(0.0f);
    mMob->setYya(0.0f);
    mMob->setZza(0.0f);
    mMob->setMoving(false);
}
