#include "game/entity/ai/TurtleGoHomeGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Turtle.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/util/Random.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/BlockPos.hpp"

TurtleGoHomeGoal::TurtleGoHomeGoal(Turtle* turtle, double speedModifier)
    : Goal(),
      mTurtle(turtle),
      mSpeedModifier(speedModifier),
      mReachedHome(false),
      mTravelTime(0)
{
}

TurtleGoHomeGoal::~TurtleGoHomeGoal()
{
}

bool TurtleGoHomeGoal::canUse()
{
    if (mTurtle->isBaby())
    {
        return false;
    }

    if (mTurtle->hasEgg() && !mTurtle->hasGivenUpOnEgg())
    {
        return true;
    }

    if (mTurtle->getRandom()->nextInt(700) != 0)
    {
        return false;
    }

    BlockPos home = mTurtle->getHomePos();
    return mTurtle->distanceToSqr(home) >= 4096.0;
}

void TurtleGoHomeGoal::start()
{
    mTurtle->setGoingHome(true);
    mTravelTime = 0;
    mReachedHome = false;
}

void TurtleGoHomeGoal::stop()
{
    mTurtle->setGoingHome(false);
}

bool TurtleGoHomeGoal::canContinueToUse()
{
    BlockPos home = mTurtle->getHomePos();
    return mTurtle->distanceToSqr(home) >= 16.0
        && !mReachedHome
        && mTravelTime < 601;
}

void TurtleGoHomeGoal::tick()
{
    BlockPos home = mTurtle->getHomePos();

    if (mTurtle->distanceToSqr(home) <= 256.0)
    {
        ++mTravelTime;
        if (mTurtle->getNavigation()->isDone())
        {
            mTurtle->getNavigation()->stop();
            mTurtle->getNavigation()->moveTo((double)home.x, (double)home.y, (double)home.z, mSpeedModifier);
            return;
        }
    }

    if (!mTurtle->closeToNextPos() && !mTurtle->getNavigation()->isDone())
    {
        return;
    }

    Vec3* position = RandomPos::getPosTowards(
        boost::static_pointer_cast<PathfinderMob>(mTurtle->shared_from_this()), 16, 3,
        Vec3::newTemp((double)home.x, (double)home.y, (double)home.z), 0.3141592741012573);

    if (position == NULL)
    {
        position = RandomPos::getPosTowards(
            boost::static_pointer_cast<PathfinderMob>(mTurtle->shared_from_this()), 8, 7,
            Vec3::newTemp((double)home.x, (double)home.y, (double)home.z));
        if (position == NULL)
        {
            mReachedHome = true;
            return;
        }
    }

    mTurtle->getNavigation()->moveTo(position->x, position->y, position->z, mSpeedModifier);
}

std::wstring TurtleGoHomeGoal::getName()
{
    return std::wstring(L"Turtle Go Home");
}
