#include "game/entity/ai/TurtleGoHomeGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Turtle.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/BlockPos.hpp"

class PathfinderMob;

namespace RandomPos
{
extern Vec3* getPosTowards(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, Vec3* target)
    __asm__("getPosTowards__9RandomPosSFRCQ2_5boost34shared_ptr__tm__16_13PathfinderMobiT2P4Vec3");
extern Vec3* getPosTowards(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, Vec3* target, double maxAngleDelta)
    __asm__("getPosTowards__9RandomPosSFRCQ2_5boost34shared_ptr__tm__16_13PathfinderMobiT2P4Vec3d");
}

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
    double distance = mTurtle->distanceToSqr(home);
    PathNavigation* navigation = mTurtle->getNavigation();

    if (distance <= 256.0)
    {
        ++mTravelTime;
        navigation->tick();
        if (navigation->isDone())
        {
            navigation->stop();
            navigation->moveTo((double)home.x, (double)home.y, (double)home.z, mSpeedModifier);
            return;
        }
    }

    if (!mTurtle->closeToNextPos())
    {
        navigation->tick();
        if (!navigation->isDone())
        {
            return;
        }
    }

    boost::shared_ptr<PathfinderMob> mob =
        boost::static_pointer_cast<PathfinderMob>(mTurtle->shared_from_this());
    Vec3* target = Vec3::newTemp((double)home.x, (double)home.y, (double)home.z);
    Vec3* position = RandomPos::getPosTowards(mob, 16, 3, target, 0.3141592741012573);

    if (position == NULL)
    {
        position = RandomPos::getPosTowards(mob, 8, 7, target);
        if (position == NULL)
        {
            mReachedHome = true;
            return;
        }
    }

    navigation->moveTo(position->x, position->y, position->z, mSpeedModifier);
}

std::wstring TurtleGoHomeGoal::getName()
{
    return std::wstring(L"Turtle_Go_Home");
}
