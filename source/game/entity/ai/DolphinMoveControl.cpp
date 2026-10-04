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
    Mob* mob = mMob;
    if (mob->isInWater())
    {
        mob->mYd += 0.005;
    }

    if (mOperation == MOVE_TO && !mob->getNavigation()->isDone())
    {
        mob->setMoving(true);

        double dx = mWantedX - mob->mX;
        double dz = mWantedZ - mob->mZ;
        double dy = mWantedY + 0.5 - mob->mY;
        if (dy * dy + dx * dx + dz * dz < MIN_SPEED_SQR)
        {
            mMob->setZza(0.0f);
            return;
        }

        float wantedYRot = (float)atan2(dz, dx) * 57.295776f - 90.0f;
        float yRot = rotlerp(mob->mYRot, wantedYRot, mMaxXRotChange);
        mob->mYRot = yRot;
        mob->mYHeadRot = yRot;
        mob->mYBodyRot = yRot;

        AttributeInstance* movementSpeed = mob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
        float speed = (float)(mSpeedModifier * movementSpeed->getValue());

        if (mob->isInWater())
        {
            mob->setSpeed(speed * 0.02f);

            float horizontal = (float)Mth::sqrt(dx * dx + dz * dz);
            float wantedXRot = Mth::wrapDegrees(-atan2f((float)dy, horizontal * 57.295776f));
            wantedXRot = Mth::clamp(wantedXRot, -mMaxYRotChange, mMaxYRotChange);
            float xRot = rotlerp(mob->mXRot, wantedXRot, 5.0f);
            mob->mXRot = xRot;
            float zza = Mth::cos(xRot * 0.017453292f) * speed;
            mob->setZza(zza);
            mob->setYya((float)dy * speed);
        }
        else
        {
            mob->setSpeed(speed * 0.1f);
        }

        return;
    }

    mob->setSpeed(0.0f);
    mob->setXxa(0.0f);
    mob->setYya(0.0f);
    mob->setZza(0.0f);
    mob->setMoving(false);
}
