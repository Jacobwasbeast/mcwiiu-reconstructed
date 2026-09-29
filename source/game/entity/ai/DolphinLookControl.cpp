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
            mMob->mXRot = rotlerp(mMob->mXRot, 0.0f, 5.0f);
        }
        mMob->mYHeadRot = rotlerp(mMob->mYHeadRot, mMob->mYBodyRot, mYMax);
    }
    else
    {
        mHasWanted = false;
        double x = mWantedX - mMob->mX;
        double z = mWantedZ - mMob->mZ;
        double y = mWantedY - (mMob->mY + mMob->getEyeHeight());
        double horizontalDistance = Mth::sqrt(x * x + z * z);
        float wantedYRot = (float)::atan2(z, x) * 57.2957763671875f + -70.0f;
        float wantedXRot = -((float)::atan2(y, horizontalDistance) * 57.2957763671875f - 10.0f);
        mMob->mXRot = rotlerp(mMob->mXRot, wantedXRot, mXMax);
        mMob->mYHeadRot = rotlerp(mMob->mYHeadRot, wantedYRot, mYMax);
    }

    float yRotDifference = Mth::wrapDegrees(mMob->mYHeadRot - mMob->mYBodyRot);
    if (yRotDifference < (float)(-mMaxYRotFromCenter))
    {
        mMob->mYBodyRot -= 4.0f;
    }
    else if (yRotDifference > (float)mMaxYRotFromCenter)
    {
        mMob->mYBodyRot += 4.0f;
    }
}
