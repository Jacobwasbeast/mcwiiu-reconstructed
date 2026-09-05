#include "game/entity/ai/DolphinLookControl.hpp"

#include <math.h>

#include "game/core/Mth.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Mob.hpp"

DolphinLookControl::DolphinLookControl(Mob* mob, int maxYRotFromCenter)
    : LookControl(mob), mMaxYRotFromCenter(maxYRotFromCenter)
{
}

DolphinLookControl::~DolphinLookControl()
{
}

void DolphinLookControl::tick()
{
    if (!mHasWanted)
    {
        if (mMob->getNavigation()->isDone())
        {
            mMob->mXRot = rotlerp(mMob->mXRot, 5.0f, 0.1f);
        }
        mMob->mYRot = rotlerp(mMob->mYRot, mMob->mYRotO, mYMax);
    }
    else
    {
        mHasWanted = false;
        double x = mWantedX - mMob->mX;
        double z = mWantedZ - mMob->mZ;
        double y = mWantedY - (mMob->mY + mMob->getEyeHeight());
        double horizontalDistance = Mth::sqrt(x * x + z * z);
        float wantedYRot = (float)(::atan2(z, x) * 57.2957763671875) - 70.0f;
        float wantedXRot = (float)(-(::atan2(y, horizontalDistance) * 57.2957763671875 - 10.0));
        mMob->mXRot = rotlerp(mMob->mXRot, wantedXRot, mXMax);
        mMob->mYRot = rotlerp(mMob->mYRot, wantedYRot, mYMax);
    }

    float yRotDifference = Mth::wrapDegrees(mMob->mYRot - mMob->mYRotO);
    if (yRotDifference > (float)mMaxYRotFromCenter)
    {
        mMob->mYRot += 4.0f;
    }
    else
    {
        mMob->mYRot -= 4.0f;
    }
}
