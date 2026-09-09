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
            if (speed < 0.08f)
            {
                speed = 0.08f;
            }
            mTurtle->setSpeed(speed);
        }
    }
    else if (mTurtle->mOnGround)
    {
        float speed = mTurtle->getSpeed() * 0.5f;
        if (speed < 0.06f)
        {
            speed = 0.06f;
        }
        mTurtle->setSpeed(speed);
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
    float wantedYRot = (float)(atan2(z, x) * 57.2957763671875) - 90.0f;
    mTurtle->mYRot = rotlerp(mTurtle->mYRot, wantedYRot, 30.0f);
    mTurtle->mYBodyRot = mTurtle->mYRot;

    float wantedSpeed = (float)(mSpeedModifier
        * mTurtle->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue());
    mTurtle->setSpeed((wantedSpeed - mTurtle->getSpeed()) * 0.125f + mTurtle->getSpeed());

    double climb = y / distance;
    mTurtle->mYd += (double)mTurtle->getSpeed() * climb * 0.1;

    if ((double)mMob->mMaxUpStep < climb)
    {
        float horizontalLimit = 1.0f;
        if (horizontalLimit < mMob->mBbWidth)
        {
            horizontalLimit = mMob->mBbWidth;
        }
        if (x * x + z * z < (double)horizontalLimit)
        {
            static_cast<JumpControl*>(mMob->getJumpControl())->jump();
            mOperation = JUMPING;
        }
    }
}
