#include "game/entity/ai/TurtleMoveControl.hpp"

#include <math.h>

#include "game/core/AttributeInstance.hpp"
#include "game/core/JumpControl.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Turtle.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"

TurtleMoveControl::TurtleMoveControl(Turtle* turtle)
    : MoveControl(turtle), mTurtle(turtle)
{
}

TurtleMoveControl::~TurtleMoveControl()
{
}

void TurtleMoveControl::updateSpeed()
{
    if (mTurtle->isInWater())
    {
        mTurtle->mYd += 0.005;
        BlockPos homePos = mTurtle->getHomePos();
        if (mTurtle->distanceToSqr(homePos) > 256.0)
        {
            float speed = mTurtle->getSpeed() * 0.5f;
            mTurtle->setSpeed(speed - 0.08f >= 0.0f ? speed : 0.08f);
        }
    }
    else if (mTurtle->mOnGround)
    {
        float speed = mTurtle->getSpeed() * 0.5f;
        mTurtle->setSpeed(speed - 0.06f >= 0.0f ? speed : 0.06f);
    }
}

void TurtleMoveControl::tick()
{
    if (mTurtle->isLayingEgg())
    {
        mTurtle->setSpeed(0.0f);
        return;
    }

    updateSpeed();

    if (mOperation == JUMPING)
    {
        mMob->setSpeed((float)(mSpeedModifier
            * mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue()));
        if (mMob->mOnGround)
        {
            mOperation = WAIT;
        }
    }
    else if (mOperation != MOVE_TO || mTurtle->getNavigation()->isDone())
    {
        mTurtle->setSpeed(0.0f);
        return;
    }

    double x = mWantedX - mTurtle->mX;
    double y = mWantedY - mTurtle->mY;
    double z = mWantedZ - mTurtle->mZ;
    double distance = Mth::sqrt(x * x + y * y + z * z);
    float wantedYRot = (float)atan2(z, x) * 57.295776f - 90.0f;
    float yRot = rotlerp(mTurtle->mYRot, wantedYRot, 30.0f);
    mTurtle->mYRot = yRot;
    mTurtle->mYBodyRot = yRot;

    float wantedSpeed = (float)(mSpeedModifier
        * mTurtle->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue());
    mTurtle->setSpeed(mTurtle->getSpeed() + (wantedSpeed - mTurtle->getSpeed()) * 0.125f);

    double climb = y / distance;
    mTurtle->mYd += (double)mTurtle->getSpeed() * climb * 0.1;

    if ((double)mMob->mMaxUpStep < climb)
    {
        float width = mMob->mBbWidth;
        float horizontalLimit = 1.0f - width >= 0.0f ? 1.0f : width;
        if (x * x + z * z < (double)horizontalLimit)
        {
            static_cast<JumpControl*>(mMob->getJumpControl())->jump();
            mOperation = JUMPING;
        }
    }
}
