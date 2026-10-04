#include "game/entity/ai/FlyingPathNavigation.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
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
    return (canFloat() && isInLiquid()) || !mMob->isPassenger();
}

Vec3* FlyingPathNavigation::getTempMobPos()
{
    return Vec3::newTemp(mMob->mX, mMob->mY, mMob->mZ);
}

Path* FlyingPathNavigation::createPath(boost::shared_ptr<Entity> entity)
{
    return createPath(BlockPos(entity));
}

void FlyingPathNavigation::tick()
{
    ++mTick;
    if (mHasDelayedRecomputation)
        recomputePath();

    if (!isDone())
    {
        if (canUpdatePath())
        {
            updatePath();
        }
        else
        {
            if (mPath != NULL && mPath->getIndex() < mPath->getSize())
            {
                Vec3* next = mPath->getPos(mMob->shared_from_this(), mPath->getIndex());
                if (Mth::floor(mMob->mX) == Mth::floor(next->x) &&
                    Mth::floor(mMob->mY) == Mth::floor(next->y) &&
                    Mth::floor(mMob->mZ) == Mth::floor(next->z))
                    mPath->setIndex(mPath->getIndex() + 1);
            }
        }
        debugPathfinding();

        if (!isDone())
        {
            Vec3* current = mPath->currentPos(mMob->shared_from_this());
            mMob->getMoveControl()->setWantedPosition(current->x, current->y, current->z,
                                                      mSpeedModifier);
        }
    }
}

bool FlyingPathNavigation::canMoveDirectly(Vec3* start, Vec3* end, int, int, int)
{
    const int x = Mth::floor(start->x);
    const int y = Mth::floor(start->y);
    const int z = Mth::floor(start->z);
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
    double xDistance = (x - start->x);
    double yDistance = (y - start->y);
    double zDistance = (z - start->z);
    if (xStep >= 0.0)
        xDistance += 1.0;
    if (yStep >= 0.0)
        yDistance += 1.0;
    if (zStep >= 0.0)
        zDistance += 1.0;
    xDistance /= xStep;
    yDistance /= yStep;
    zDistance /= zStep;
    const int xDirection = xStep < 0.0 ? -1 : 1;
    const int yDirection = yStep < 0.0 ? -1 : 1;
    const int zDirection = zStep < 0.0 ? -1 : 1;
    int xRemaining = Mth::floor(end->x) - x;
    int yRemaining = Mth::floor(end->y) - y;
    int zRemaining = Mth::floor(end->z) - z;
    while (true)
    {
        if (xRemaining * xDirection < 1 && yRemaining * yDirection < 1 &&
            zRemaining * zDirection < 1)
        {
            return true;
        }
        if (yDistance < xDistance)
        {
            if (zDistance < yDistance)
            {
                zDistance += 1.0 / std::fabs(zStep);
                zRemaining -= zDirection;
            }
            else
            {
                yDistance += 1.0 / std::fabs(yStep);
                yRemaining -= yDirection;
            }
        }
        else if (zDistance <= xDistance)
        {
            zDistance += 1.0 / std::fabs(zStep);
            zRemaining -= zDirection;
        }
        else
        {
            xDistance += 1.0 / std::fabs(xStep);
            xRemaining -= xDirection;
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
