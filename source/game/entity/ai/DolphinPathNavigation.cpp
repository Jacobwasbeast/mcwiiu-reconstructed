#include "game/entity/ai/DolphinPathNavigation.hpp"

#include <cmath>

#include "game/core/Dolphin.hpp"
#include "game/core/Path.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

DolphinPathNavigation::DolphinPathNavigation(Dolphin* dolphin, Level* level)
    : WaterBoundPathNavigation(dolphin, level, true, true)
{
}

DolphinPathNavigation::~DolphinPathNavigation()
{
}

bool DolphinPathNavigation::canUpdatePath()
{
    return true;
}

bool DolphinPathNavigation::isStableDestination(const BlockPos& pos)
{
    if (mMob->isInWaterOrBubble())
        return WaterBoundPathNavigation::isStableDestination(pos);
    return !mLevel->getBlock(pos)->isPathfindable(mLevel, pos);
}

void DolphinPathNavigation::updatePath()
{
    if (mMob->isInWaterOrBubble())
    {
        WaterBoundPathNavigation::updatePath();
        return;
    }

    boost::shared_ptr<Entity> entity = mMob->shared_from_this();
    Vec3* mobPos = getTempMobPos();
    int index = mPath->getIndex();
    int size = mPath->getSize();
    const double waypointDistance = 0.36;
    const double waypointHeight = 0.6;

    while (index < size)
    {
        Vec3* pathPos = mPath->getPos(entity, index);
        Vec3* horizontalPos = Vec3::newTemp(mobPos->x, pathPos->y, mobPos->z);
        if (horizontalPos->distanceToSqr(pathPos) < waypointDistance &&
            std::fabs(mobPos->y - pathPos->y) < waypointHeight)
            mPath->setIndex(index + 1);
        ++index;
    }

    int width = static_cast<int>(std::ceil(mMob->mBbWidth));
    int height = static_cast<int>(mMob->mBbHeight) + 1;
    int best = mPath->getIndex() + 6;
    if (size - 1 < best)
        best = size - 1;

    index = mPath->getIndex();
    const double maxDistance = 36.0;
    while (index < best)
    {
        Vec3* pathPos = mPath->getPos(entity, best);
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
    if (mMob->isInWaterOrBubble())
        return PathNavigation::createPath(pos);
    return PathNavigation::createDirectPath(pos);
}
