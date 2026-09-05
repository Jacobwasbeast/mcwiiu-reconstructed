#include "game/entity/ai/FlyingPathNavigation.hpp"

#include <cmath>

#include "game/core/Path.hpp"
#include "game/core/PathFinder.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"

FlyingPathNavigation::FlyingPathNavigation(Mob* mob, Level* level)
    : PathNavigation(mob, level), mNodeEvaluator(mob)
{
    mPathFinder = createPathFinder();
}

FlyingPathNavigation::~FlyingPathNavigation()
{
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
    return !((!canFloat() || !isInLiquid()) && mMob->isNoGravity());
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
            Vec3* mobPos = getTempMobPos();
            if (static_cast<int>(std::floor(mobPos->x)) == static_cast<int>(std::floor(next->x)) &&
                static_cast<int>(std::floor(mobPos->y)) == static_cast<int>(std::floor(next->y)) &&
                static_cast<int>(std::floor(mobPos->z)) == static_cast<int>(std::floor(next->z)))
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
        mMob->getMoveControl()->setWantedPosition(current->x, current->y, current->z, mSpeedModifier);
    }
}

bool FlyingPathNavigation::canMoveDirectly(Vec3* a, Vec3* b, int x, int y, int z)
{
    (void)x;
    (void)y;
    (void)z;

    const double dx = b->x - a->x;
    const double dy = b->y - a->y;
    const double dz = b->z - a->z;
    const double lengthSquared = dx * dx + dy * dy + dz * dz;
    if (lengthSquared < 1.0e-8)
        return false;

    const double length = std::sqrt(lengthSquared);
    const double stepX = dx / length;
    const double stepY = dy / length;
    const double stepZ = dz / length;
    int currentX = static_cast<int>(std::floor(a->x));
    int currentY = static_cast<int>(std::floor(a->y));
    int currentZ = static_cast<int>(std::floor(a->z));
    const int targetX = static_cast<int>(std::floor(b->x));
    const int targetY = static_cast<int>(std::floor(b->y));
    const int targetZ = static_cast<int>(std::floor(b->z));
    const int directionX = stepX < 0.0 ? -1 : 1;
    const int directionY = stepY < 0.0 ? -1 : 1;
    const int directionZ = stepZ < 0.0 ? -1 : 1;
    double distanceX = ((stepX < 0.0 ? currentX : currentX + 1.0) - a->x) / stepX;
    double distanceY = ((stepY < 0.0 ? currentY : currentY + 1.0) - a->y) / stepY;
    double distanceZ = ((stepZ < 0.0 ? currentZ : currentZ + 1.0) - a->z) / stepZ;

    while (true)
    {
        if ((targetX - currentX) * directionX < 1 &&
            (targetY - currentY) * directionY < 1 &&
            (targetZ - currentZ) * directionZ < 1)
            return true;

        if (distanceY < distanceX)
        {
            if (distanceZ <= distanceY)
            {
                distanceZ += 1.0 / std::fabs(stepZ);
                currentZ += directionZ;
            }
            else
            {
                distanceY += 1.0 / std::fabs(stepY);
                currentY += directionY;
            }
        }
        else if (distanceZ <= distanceX)
        {
            distanceZ += 1.0 / std::fabs(stepZ);
            currentZ += directionZ;
        }
        else
        {
            distanceX += 1.0 / std::fabs(stepX);
            currentX += directionX;
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
    return mLevel->getBlock(pos)->isPathfindable(mLevel, pos);
}

NodeEvaluator* FlyingPathNavigation::getNodeEvaluator()
{
    return &mNodeEvaluator;
}
