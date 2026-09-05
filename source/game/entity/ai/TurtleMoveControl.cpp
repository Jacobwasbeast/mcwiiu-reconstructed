#include "game/entity/ai/TurtleMoveControl.hpp"

#include <math.h>

#include "game/core/AttributeInstance.hpp"
#include "game/core/JumpControl.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/Turtle.hpp"

TurtleMoveControl::TurtleMoveControl(Turtle* turtle)
    : MoveControl(turtle), mTurtle(turtle)
{
}

TurtleMoveControl::~TurtleMoveControl()
{
}

void TurtleMoveControl::updateSpeed()
{
    AttributeInstance* movementSpeed = mTurtle->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
    double speed = movementSpeed->getValue();

    if (!mTurtle->isGoingHome())
    {
        if (mTurtle->isInWater())
        {
            double adjustedSpeed = speed * 0.5;
            if (adjustedSpeed < 0.06)
                adjustedSpeed = 0.06;
            mTurtle->setSpeed((float)adjustedSpeed);
        }
        return;
    }

    mTurtle->mY += 0.005;
    BlockPos homePos = mTurtle->getHomePos();
    if (mTurtle->distanceToSqr(homePos) > 256.0)
    {
        double adjustedSpeed = speed * 0.5;
        if (adjustedSpeed < 0.08)
            adjustedSpeed = 0.08;
        mTurtle->setSpeed((float)adjustedSpeed);
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
        AttributeInstance* movementSpeed = mTurtle->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
        mTurtle->setSpeed((float)(mSpeedModifier * movementSpeed->getValue()));
        if (mTurtle->isInWater())
            mOperation = WAIT;
    }

    if (mOperation == MOVE_TO || mOperation == JUMPING)
    {
        if (mOperation == MOVE_TO)
        {
            mTurtle->getNavigation()->stop();
            if (mTurtle->getNavigation()->isDone())
            {
                mTurtle->setSpeed(0.0f);
                return;
            }
        }

        double x = mWantedX - mTurtle->mX;
        double z = mWantedZ - mTurtle->mZ;
        double y = mWantedY - mTurtle->mY;
        double distance = Mth::sqrt(x * x + y * y + z * z);
        float wantedYRot = (float)(atan2(z, x) * 57.2957763671875) - 90.0f;
        mTurtle->mYRot = rotlerp(mTurtle->mYRot, wantedYRot, 30.0f);
        mTurtle->mYBodyRot = mTurtle->mYRot;

        AttributeInstance* movementSpeed = mTurtle->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
        double currentSpeed = movementSpeed->getValue();
        mTurtle->setSpeed((float)((mSpeedModifier * currentSpeed - currentSpeed) * 0.125 + currentSpeed));

        float horizontalLimit = mTurtle->mBbWidth * mTurtle->mBbWidth;
        if (horizontalLimit < 1.0f)
            horizontalLimit = 1.0f;
        if (mTurtle->mMaxUpStep < y / distance && x * x + z * z < horizontalLimit)
        {
            static_cast<JumpControl*>(mTurtle->getJumpControl())->jump();
            mOperation = JUMPING;
        }
        return;
    }

    mTurtle->setSpeed(0.0f);
}
