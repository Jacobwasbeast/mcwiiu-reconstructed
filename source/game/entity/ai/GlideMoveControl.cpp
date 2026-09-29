#include "game/entity/ai/GlideMoveControl.hpp"

#include <math.h>

#include "game/core/AttributeInstance.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/util/Vec3.hpp"

GlideMoveControl::GlideMoveControl(Mob* mob, float startSpeed, float speedWhenTurning)
    : MoveControl(mob), mStartSpeed(startSpeed), mSpeedWhenTurning(speedWhenTurning), mSpeed(startSpeed)
{
}

GlideMoveControl::~GlideMoveControl()
{
}

void GlideMoveControl::tick()
{
    if (mOperation != MOVE_TO)
    {
        return;
    }

    Mob* mob = mMob;
    double dz = mWantedZ - mob->mZ;
    double dx = mWantedX - mob->mX;
    double dy = mWantedY - mob->mY;
    double horizontalDistance = Mth::sqrt(dx * dx + dz * dz);

    if (mob->mHorizontalCollision)
    {
        mob->mYRot += 180.0f;
        mSpeed = mStartSpeed;
    }

    double horizontalScale = 1.0;
    if (horizontalDistance != 0.0)
    {
        horizontalScale = 1.0 - Mth::abs(dy * 0.7) / horizontalDistance;
    }

    dx *= horizontalScale;
    dz *= horizontalScale;
    double horizontalLength = Mth::sqrt(dx * dx + dz * dz);
    double distance = Mth::sqrt(dy * dy + dx * dx + dz * dz);
    float oldYRot = mob->mYRot;
    float wantedYRot = Mth::wrapDegrees((float)atan2(dz, dx) * 57.2957763671875f);
    float yRot = Mth::approachDegrees(Mth::wrapDegrees(mob->mYRot + 90.0f), wantedYRot, 4.0f) - 90.0f;
    mob->mYBodyRot = yRot;
    mob->mYRot = yRot;

    if (Mth::degreesDifferenceAbs(oldYRot, mob->mYRot) >= 3.0f)
    {
        mSpeed = Mth::approach(mSpeed, (float)(mSpeedWhenTurning * mSpeedModifier), 0.025f);
    }
    else
    {
        float movementSpeed = (float)mob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue();
        mSpeed = Mth::approach(mSpeed, (float)(movementSpeed * mSpeedModifier),
                               0.005f * (float)(0.6999999225139618 / (double)mSpeed));
    }

    float wantedXRot = -((float)atan2(-dy, horizontalLength) * 57.29578f);
    LookControl* lookControl = static_cast<LookControl*>(mob->getLookControl());
    lookControl->setLookRotation(Vec3::newTemp((double)wantedXRot, (double)mob->mYRot, 0.0),
                                  60.0f, 20.0f);

    float yawRadians = (mob->mYRot + 90.0f) * 0.01745329238474369f;
    float pitchRadians = wantedXRot * 0.01745329238474369f;
    float cosYaw = cosf(yawRadians);
    float sinYaw = sinf(yawRadians);
    float sinPitch = sinf(pitchRadians);
    mob->mXd += ((double)(mSpeed * cosYaw * Mth::abs((float)(dx / distance))) - mob->mXd) * 0.2;
    mob->mZd += ((double)(mSpeed * sinYaw * Mth::abs((float)(dz / distance))) - mob->mZd) * 0.2;
    mob->mYd += ((double)(mSpeed * sinPitch * Mth::abs((float)(dy / distance))) - mob->mYd) * 0.2;
    mob->mFallDistance = 0.0f;
}
