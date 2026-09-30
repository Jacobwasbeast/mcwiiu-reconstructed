#include "game/entity/ai/DolphinPathNavigation.hpp"

#include <cmath>

#include "game/core/Dolphin.hpp"
#include "game/core/Path.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

DolphinPathNavigation::DolphinPathNavigation(Dolphin* dolphin, Level* level)
    : WaterBoundPathNavigation(dolphin, level, true, true)
{
}

bool DolphinPathNavigation::canUpdatePath()
{
    return true;
}

bool DolphinPathNavigation::isStableDestination(const BlockPos& pos)
{
    if (mMob->isInWater())
        return WaterBoundPathNavigation::isStableDestination(pos);
    return !mLevel->getBlockState(pos)->isSolid();
}

void DolphinPathNavigation::updatePath()
{
    if (mMob->isInWater())
    {
        WaterBoundPathNavigation::updatePath();
        return;
    }

    Vec3* mobPos = getTempMobPos();
    int index = mPath->getIndex();
    int size = mPath->getSize();
    const double waypointDistance = 0.36;
    const double waypointHeight = 0.6;

    while (index < size)
    {
        Vec3* pathPos = mPath->getPos(mMob->shared_from_this(), index);
        Vec3* horizontalPos = Vec3::newTemp(mobPos->x, pathPos->y, mobPos->z);
        if (horizontalPos->distanceToSqr(pathPos) < waypointDistance &&
            std::fabs(mobPos->y - pathPos->y) < waypointHeight)
            mPath->setIndex(index + 1);
        ++index;
        size = mPath->getSize();
    }

    int width = static_cast<int>(ceilf(mMob->mBbWidth));
    int height = static_cast<int>(mMob->mBbHeight) + 1;
    int best = mPath->getIndex() + 6;
    if (size - 1 < best)
        best = size - 1;

    index = mPath->getIndex();
    const double maxDistance = 36.0;
    while (index < best)
    {
        Vec3* pathPos = mPath->getPos(mMob->shared_from_this(), best);
        if (pathPos->distanceToSqr(mobPos) <= maxDistance &&
            canMoveDirectly(mobPos, pathPos, width, height, width))
        {
            mPath->setIndex(best);
            doStuckDetection(mobPos);
            return;
        }
        --best;
        index = mPath->getIndex();
    }

    doStuckDetection(mobPos);
}

Path* DolphinPathNavigation::createPath(const BlockPos& pos)
{
    if (mMob->isInWater())
        return PathNavigation::createPath(pos);
    return PathNavigation::createDirectPath(pos);
}
