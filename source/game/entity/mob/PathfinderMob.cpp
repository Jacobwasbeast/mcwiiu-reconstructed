#include "game/entity/mob/PathfinderMob.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/Path.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/EntityType.hpp"
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/util/Vec3.hpp"

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
    if (!isLeashed() || getLeashHolder() == NULL || getLeashHolder()->mLevel != mLevel)
    {
        return;
    }

    boost::shared_ptr<Entity> holder = getLeashHolder();
    restrictTo(BlockPos(static_cast<int>(holder->mX), static_cast<int>(holder->mY), static_cast<int>(holder->mZ)), 5);
    float distance = distanceTo(holder);

    boost::shared_ptr<TamableAnimal> tamable;
    if ((GetType() & EntityType::TAMABLE_ANIMAL) == EntityType::TAMABLE_ANIMAL)
    {
        tamable = boost::dynamic_pointer_cast<TamableAnimal>(shared_from_this());
    }

    if (tamable != NULL && tamable->isSitting())
    {
        if (distance > 10.0f)
        {
            dropLeash(true, true);
        }
        return;
    }

    onLeashDistance(distance);
    if (distance > 10.0f)
    {
        dropLeash(true, true);
        mGoalSelector.disableControlFlag(1);
    }
    else if (distance > 6.0f)
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
        mGoalSelector.enableControlFlag(1);
        Vec3* offset = Vec3::newTemp(holder->mX - mX, holder->mY - mY, holder->mZ - mZ)->normalize()->scale(std::max(distance - 2.0f, 0.0f));
        getNavigation()->moveTo(mX + offset->x, mY + offset->y, mZ + offset->z, followLeashSpeed());
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
