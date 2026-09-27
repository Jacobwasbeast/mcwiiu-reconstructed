#include "game/entity/mob/PathfinderMob.hpp"

#include <cmath>

#include "game/core/Path.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/Level.hpp"

PathfinderMob::PathfinderMob(Level* level)
    : Mob(level),
      mPath(NULL),
      mPathTick(0),
      mPathOwner(NULL),
      mPathValid(false),
      mPathDistance(0),
      mRestrictionCenter(),
      mRestrictionRadius(-1.0f),
      mWalkTargetValue(0.0f)
{
    mRestrictionCenter = BlockPos(0, 0, 0);
    mWalkTargetValue = 0.0f;
}

PathfinderMob::~PathfinderMob()
{
    delete mPath;
}

double PathfinderMob::getWalkTargetValue(const BlockPos&)
{
    return 0.0;
}

bool PathfinderMob::checkSpawnRules()
{
    if (!Mob::checkSpawnRules())
    {
        return false;
    }

    BlockPos pos(mX, mY, mZ);
    return getWalkTargetValue(pos) >= 0.0;
}

bool PathfinderMob::isPathFinding()
{
    PathNavigation* navigation = getNavigation();
    return navigation != NULL && !navigation->isDone();
}

bool PathfinderMob::isWithinRestriction()
{
    return isWithinRestriction(BlockPos(shared_from_this()));
}

bool PathfinderMob::isWithinRestriction(const BlockPos& pos)
{
    if (mRestrictionRadius != -1.0f)
    {
        return mRestrictionCenter.distSqr(pos) <
            (double)mRestrictionRadius * (double)mRestrictionRadius;
    }

    return true;
}

void PathfinderMob::restrictTo(const BlockPos& pos, int radius)
{
    mRestrictionCenter = pos;
    mRestrictionRadius = (float)radius;
}

BlockPos PathfinderMob::getRestrictCenter()
{
    return mRestrictionCenter;
}

double PathfinderMob::getRestrictRadius()
{
    return (double)mRestrictionRadius;
}

void PathfinderMob::clearRestriction()
{
    mRestrictionRadius = -1.0f;
}

bool PathfinderMob::hasRestriction()
{
    return mRestrictionRadius != -1.0f;
}

void PathfinderMob::tickLeash()
{
    Entity::tickLeash();

    if (!isLeashed())
    {
        return;
    }

    boost::shared_ptr<Entity> holder = getLeashHolder();
    if (!holder)
    {
        return;
    }

    if (holder->GetType() == GetType())
    {
        dropLeash(true, true);
        return;
    }

    double distance = distanceTo(holder);
    if (distance > 10.0)
    {
        dropLeash(true, true);
        return;
    }

    if (distance > 6.0)
    {
        Vec3* direction = Vec3::newTemp(holder->mX - mX,
                                         holder->mY - mY,
                                         holder->mZ - mZ);
        direction->normalize();
        double amount = (float)(distance - 2.0);
        if (amount < 0.0)
        {
            amount = 0.0;
        }
        Vec3* movement = direction->scale(amount);
        MoveControl* moveControl = static_cast<MoveControl*>(getMoveControl());
        moveControl->setWantedPosition(mX + movement->x,
                                       mY + movement->y,
                                       mZ + movement->z,
                                       followLeashSpeed());
    }
    else if (distance > 2.0)
    {
        double x = (holder->mX - mX) / distance;
        double y = (holder->mY - mY) / distance;
        double z = (holder->mZ - mZ) / distance;
        mXd += x * std::fabs(x) * 0.4;
        mYd += y * std::fabs(y) * 0.4;
        mZd += z * std::fabs(z) * 0.4;
    }
    else
    {
        PathNavigation* navigation = getNavigation();
        if (navigation != NULL)
        {
            navigation->stop();
        }
    }
}

void PathfinderMob::onLeashDistance(float)
{
}

double PathfinderMob::followLeashSpeed()
{
    return 0.9;
}

bool PathfinderMob::couldWander()
{
    return mTickCount < 100 || Entity::couldWander();
}
