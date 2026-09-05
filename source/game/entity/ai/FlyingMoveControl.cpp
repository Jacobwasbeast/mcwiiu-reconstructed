#include "game/entity/ai/FlyingMoveControl.hpp"

#include "game/core/AttributeInstance.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"

FlyingMoveControl::FlyingMoveControl(Mob* mob)
    : MoveControl(mob)
{
}

FlyingMoveControl::~FlyingMoveControl()
{
}

void FlyingMoveControl::tick()
{
    if (mOperation != MOVE_TO)
    {
        mMob->setNoGravity(false);
        mMob->setYya(0.0f);
        mMob->setXxa(0.0f);
        return;
    }

    mOperation = WAIT;
    mMob->setNoGravity(true);

    double xDistance = mWantedX - mMob->mX;
    double zDistance = mWantedZ - mMob->mZ;
    double yDistance = mWantedY - mMob->mY;
    double horizontalDistanceSquared = xDistance * xDistance + zDistance * zDistance;

    if (2.5000003e-7 >= yDistance * yDistance + horizontalDistanceSquared)
    {
        mMob->setYya(0.0f);
        mMob->setXxa(0.0f);
        return;
    }

    float yRot = (float)((double)Mth::atan2((int)zDistance, (int)xDistance)
                         * 57.2957763671875) - 90.0f;
    mMob->mYRot = rotlerp(mMob->mYRot, yRot, 10.0f);

    Attribute* movementAttribute = mMob->mOnGround
        ? SharedMonsterAttributes::MOVEMENT_SPEED
        : SharedMonsterAttributes::FLYING_SPEED;
    float speed = (float)(mSpeedModifier * mMob->getAttribute(movementAttribute)->getValue());
    mMob->setSpeed(speed);

    double horizontalDistance = (double)Mth::sqrt(horizontalDistanceSquared);
    float xRot = (float)(-(double)Mth::atan2((int)yDistance, (int)horizontalDistance)
                         * 57.2957763671875);
    mMob->mXRot = rotlerp(mMob->mXRot, xRot, 10.0f);
    mMob->setYya(yDistance <= 0.52 ? -speed : speed);
}
