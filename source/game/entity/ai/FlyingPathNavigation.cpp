#include "game/entity/ai/FlyingPathNavigation.hpp"

#include <cmath>

#include "game/core/Path.hpp"
#include "game/core/PathFinder.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"

FlyingPathNavigation::FlyingPathNavigation(Mob* mob, Level* level)
    : PathNavigation(mob, level), mNodeEvaluator(mob)
{
    mPathFinder = createPathFinder();
}

PathFinder* FlyingPathNavigation::createPathFinder()
{
    mNodeEvaluator.setCanPassDoors(true);
    return new PathFinder(&mNodeEvaluator);
}

bool FlyingPathNavigation::canFloat()
{
    return mNodeEvaluator.isCanFloat();
}

bool FlyingPathNavigation::canUpdatePath()
{
    return !((!canFloat() || !isInLiquid()) && mMob->isPassenger());
}

Vec3* FlyingPathNavigation::getTempMobPos()
{
    return Vec3::newTemp(mMob->mX, mMob->mY, mMob->mZ);
}

Path* FlyingPathNavigation::createPath(boost::shared_ptr<Entity> entity)
{
    BlockPos pos(entity);
    return createPath(pos);
}

void FlyingPathNavigation::tick()
{
    ++mTick;
    if (mHasDelayedRecomputation)
        recomputePath();

    if (isDone())
        return;

    if (!canUpdatePath())
    {
        if (mPath != NULL && mPath->getIndex() < mPath->getSize())
        {
            boost::shared_ptr<Entity> entity = mMob->shared_from_this();
            Vec3* next = mPath->getPos(entity, mPath->getIndex());
            if (static_cast<int>(std::floor(mMob->mX)) == static_cast<int>(std::floor(next->x)) &&
                static_cast<int>(std::floor(mMob->mY)) == static_cast<int>(std::floor(next->y)) &&
                static_cast<int>(std::floor(mMob->mZ)) == static_cast<int>(std::floor(next->z)))
                mPath->setIndex(mPath->getIndex() + 1);
        }
        debugPathfinding();
    }
    else
    {
        updatePath();
        debugPathfinding();
    }

    if (!isDone())
    {
        boost::shared_ptr<Entity> entity = mMob->shared_from_this();
        Vec3* current = mPath->currentPos(entity);
        mMob->getMoveControl()->setWantedPosition(current->x, current->y, current->z,
                                                  mSpeedModifier);
    }
}

bool FlyingPathNavigation::canMoveDirectly(Vec3* start, Vec3* end, int, int, int)
{
    const double xDelta = end->x - start->x;
    const double yDelta = end->y - start->y;
    const double zDelta = end->z - start->z;
    const double lengthSquared = xDelta * xDelta + yDelta * yDelta + zDelta * zDelta;
    if (lengthSquared < 1.0e-8)
    {
        return false;
    }
    const double scale = 1.0 / std::sqrt(lengthSquared);
    const double xStep = xDelta * scale;
    const double yStep = yDelta * scale;
    const double zStep = zDelta * scale;
    int x = static_cast<int>(std::floor(start->x));
    int y = static_cast<int>(std::floor(start->y));
    int z = static_cast<int>(std::floor(start->z));
    const int endX = static_cast<int>(std::floor(end->x));
    const int endY = static_cast<int>(std::floor(end->y));
    const int endZ = static_cast<int>(std::floor(end->z));
    const int xDirection = xStep < 0.0 ? -1 : 1;
    const int yDirection = yStep < 0.0 ? -1 : 1;
    const int zDirection = zStep < 0.0 ? -1 : 1;
    double xDistance = ((x - start->x) + (xStep >= 0.0 ? 1.0 : 0.0)) / xStep;
    double yDistance = ((y - start->y) + (yStep >= 0.0 ? 1.0 : 0.0)) / yStep;
    double zDistance = ((z - start->z) + (zStep >= 0.0 ? 1.0 : 0.0)) / zStep;
    while (true)
    {
        if ((endX - x) * xDirection < 1 && (endY - y) * yDirection < 1 &&
            (endZ - z) * zDirection < 1)
        {
            return true;
        }
        if (yDistance < xDistance)
        {
            if (zDistance < yDistance)
            {
                z += zDirection;
                zDistance += 1.0 / std::fabs(zStep);
            }
            else
            {
                y += yDirection;
                yDistance += 1.0 / std::fabs(yStep);
            }
        }
        else if (zDistance <= xDistance)
        {
            z += zDirection;
            zDistance += 1.0 / std::fabs(zStep);
        }
        else
        {
            x += xDirection;
            xDistance += 1.0 / std::fabs(xStep);
        }
    }
}

void FlyingPathNavigation::setCanOpenDoors(bool value)
{
    mNodeEvaluator.setCanOpenDoors(value);
}

void FlyingPathNavigation::setCanPassDoors(bool value)
{
    mNodeEvaluator.setCanPassDoors(value);
}

void FlyingPathNavigation::setCanFloat(bool value)
{
    mNodeEvaluator.setCanFloat(value);
}

bool FlyingPathNavigation::isStableDestination(const BlockPos& pos)
{
    return mLevel->getBlockState(pos)->isTopSolidBlocking();
}

NodeEvaluator* FlyingPathNavigation::getNodeEvaluator()
{
    return &mNodeEvaluator;
}
