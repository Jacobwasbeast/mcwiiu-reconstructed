#include "game/entity/ai/TurtleTravelGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/BoundingBox.hpp"
#include "game/core/Turtle.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/Random.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

TurtleTravelGoal::TurtleTravelGoal(Turtle* turtle, double speedModifier)
    : Goal(),
      mTurtle(turtle),
      mSpeedModifier(speedModifier)
{
}

TurtleTravelGoal::~TurtleTravelGoal()
{
}

bool TurtleTravelGoal::canUse()
{
    return !mTurtle->isGoingHome()
        && (!mTurtle->hasEgg() || mTurtle->hasGivenUpOnEgg())
        && (mTurtle->isInWater() || mTurtle->isOverWater());
}

void TurtleTravelGoal::start()
{
    Random* random = mTurtle->getRandom();
    int x = random->nextInt(1025);
    int y = random->nextInt(9);
    int z = random->nextInt(1025);
    double targetY = (double)(y - 4);

    if ((double)(mTurtle->mLevel->getSeaLevel() - 1) < targetY + mTurtle->mY)
    {
        targetY = 0.0;
    }

    BlockPos travelPos(mTurtle->mX + (double)(x - 512),
                       mTurtle->mY + targetY,
                       mTurtle->mZ + (double)(z - 512));
    mTurtle->setTravelPos(travelPos);
    mTurtle->setTravelling(true);
    mReachedPosition = false;
}

void TurtleTravelGoal::tick()
{
    PathNavigation* navigation = mTurtle->getNavigation();
    if (!mTurtle->closeToNextPos())
    {
        navigation->tick();
        if (!navigation->isDone())
        {
            return;
        }
    }

    BlockPos travelPos = mTurtle->getTravelPos();
    boost::shared_ptr<PathfinderMob> mob =
        boost::static_pointer_cast<PathfinderMob>(mTurtle->shared_from_this());
    Vec3* target = Vec3::newTemp((double)travelPos.x, (double)travelPos.y, (double)travelPos.z);
    Vec3* position = RandomPos::getPosTowards(mob, 16, 3, target, 0.3141592741012573);

    if (position == NULL)
    {
        position = RandomPos::getPosTowards(mob, 8, 7, target);
        if (position == NULL)
        {
            mReachedPosition = true;
            return;
        }
    }

    int x = (int)position->x;
    int z = (int)position->z;
    if ((double)x > position->x)
    {
        --x;
    }
    if ((double)z > position->z)
    {
        --z;
    }

    BoundingBox box(x - 34, 0, z - 34, x + 34, 0, z + 34);
    if (mTurtle->mLevel->hasChunksAt(box))
    {
        navigation->moveTo(position->x, position->y, position->z, mSpeedModifier);
        return;
    }

    mReachedPosition = true;
}

bool TurtleTravelGoal::canContinueToUse()
{
    BlockPos travelPos = mTurtle->getTravelPos();
    double distance = mTurtle->distanceToSqr(travelPos);
    return distance >= 36.0
        && !mReachedPosition
        && !mTurtle->isGoingHome()
        && !mTurtle->isInLove()
        && (!mTurtle->hasEgg() || mTurtle->hasGivenUpOnEgg());
}

void TurtleTravelGoal::stop()
{
    mTurtle->setTravelling(false);
    Goal::stop();
}

std::wstring TurtleTravelGoal::getName()
{
    return std::wstring(L"Turtle Travel");
}
