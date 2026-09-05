#include "game/entity/ai/SwimMoveControl.hpp"

#include <math.h>

#include "game/core/AttributeInstance.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/util/Vec3.hpp"

SwimMoveControl::SwimMoveControl(Mob* mob)
    : MoveControl(mob)
{
    setSpeedModifier(0.5);
}

SwimMoveControl::~SwimMoveControl()
{
}

void SwimMoveControl::tick()
{
    if (!mMob->isInWater())
    {
        MoveControl::tick();
        return;
    }

    if (!hasWanted())
    {
        mMob->setSpeed(0.0f);
        mMob->setYya(0.0f);
        return;
    }

    mMob->setYya(0.0f);

    double dx = mWantedX - mMob->getPos(2.0f)->x;
    double dz = mWantedZ - mMob->getPos(2.0f)->z;
    double dy = mWantedY - mMob->getPos(2.0f)->y;
    double distance = Mth::sqrt(dx * dx + dy * dy + dz * dz);

    if (distance >= 0.0001)
    {
        double horizontal = dx * dx + dz * dz;
        double wantedYRot;

        if (horizontal > 0.1)
        {
            wantedYRot = (double)((float)(atan2(dz, dx) * 57.2957763671875) - 90.0f);
        }
        else
        {
            wantedYRot = 0.0;
        }

        mMob->mYRot = Mth::clampRotateDegrees(mMob->mYRot, (float)wantedYRot, 30.0f);
        mMob->mYBodyRot = mMob->mYRot;

        float speed = (float)mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue();
        float movementSpeed = (float)(getSpeedModifier() * (double)speed);
        mMob->setSpeed(movementSpeed);

        double horizontalDistance = Mth::sqrt(horizontal);
        double pitch = atan2(dy / distance, horizontalDistance);
        LookControl* lookControl = static_cast<LookControl*>(mMob->getLookControl());
        Vec3* rotation = Vec3::newTemp((double)((float)(pitch * -57.2957763671875)),
                                       (double)mMob->mYRot, 0.0);
        lookControl->setLookRotation(rotation, 5.0f, 10.0f);

        float radians = mMob->mXRot * 0.01745329238474369f;
        mMob->setXxa((float)(cos(radians) * movementSpeed));
        mMob->setZza((float)(-sin(radians) * movementSpeed));

        Vec3* current = mMob->getPos(2.0f);
        Vec3* target = Vec3::newTemp(current->x + dx / distance + dx / distance,
                                     current->y + (dy / distance) / distance,
                                     current->z + dz / distance + dz / distance);
        double wantedX = lookControl->getWantedX();
        double wantedY = lookControl->getWantedY();
        double wantedZ = lookControl->getWantedZ();
        Vec3* wanted = Vec3::newTemp(wantedX, wantedY, wantedZ);

        if (!lookControl->isHasWanted())
        {
            lookControl->setLookAt(target->x, target->y, target->z, 10.0f, 40.0f);
        }
        else
        {
            lookControl->setLookAt((target->x - wanted->x) * 0.125 + wanted->x,
                                   (target->y - wanted->y) * 0.125 + wanted->y,
                                   (target->z - wanted->z) * 0.125 + wanted->z,
                                   10.0f, 40.0f);
        }

        mMob->setYya(1.0f);
    }
}
