#include "game/entity/ai/DrownedPathNavigation.hpp"

#include <cmath>

#include "game/core/HitResult.hpp"
#include "game/core/Path.hpp"
#include "game/core/PathFinder.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

DrownedPathNavigation::DrownedPathNavigation(Mob* mob, Level* level)
    : GroundPathNavigation(mob, level, false),
      mAmphibiousNodeEvaluator(mob),
      mGroundPathFinder(0),
      mWaterPathFinder(0)
{
    GroundPathNavigation::createPathFinder();
    mGroundPathFinder = mPathFinder;
    createPathFinder();
    mWaterPathFinder = mPathFinder;
}

DrownedPathNavigation::~DrownedPathNavigation()
{
    mPathFinder = 0;
    delete mWaterPathFinder;
    delete mGroundPathFinder;
}

void DrownedPathNavigation::createPathFinder()
{
    mAmphibiousNodeEvaluator.setCanPassDoors(true);
    mPathFinder = new PathFinder(&mAmphibiousNodeEvaluator);
}

Path* DrownedPathNavigation::createPath(const BlockPos& pos)
{
    if (mMob->isInWaterOrBubble() && !mMob->mOnGround && isInLiquid())
    {
        mPathFinder = mWaterPathFinder;
        return PathNavigation::createPath(pos);
    }
    mPathFinder = mGroundPathFinder;
    return GroundPathNavigation::createPath(pos);
}

bool DrownedPathNavigation::canUpdatePath()
{
    return mMob->mOnGround || isInLiquid();
}

Vec3* DrownedPathNavigation::getTempMobPos()
{
    if (mMob->isInWaterOrBubble())
        return Vec3::newTemp(mMob->mX, mMob->mY + mMob->mBbHeight * 0.5, mMob->mZ);
    return GroundPathNavigation::getTempMobPos();
}

void DrownedPathNavigation::tick()
{
    if (!mMob->isInWaterOrBubble())
    {
        PathNavigation::tick();
        return;
    }
    ++mTick;
    if (mHasDelayedRecomputation)
        recomputePath();
    if (isDone())
        return;
    if (!canUpdatePath())
    {
        Vec3* pos = getTempMobPos();
        Vec3* next = mPath->getPos(mMob->shared_from_this(), mPath->getIndex());
        if (next->y < pos->y && !mMob->mOnGround)
            return;
        if (std::floor(pos->x) == std::floor(next->x) &&
            std::floor(pos->z) == std::floor(next->z))
            mPath->setIndex(mPath->getIndex() + 1);
    }
    else
    {
        PIXBeginNamedEvent(0.25f, "updatePath");
        updatePath();
        PIXEndNamedEvent();
    }
    debugPathfinding();
    if (!isDone())
    {
        Vec3* current = mPath->currentPos(mMob->shared_from_this());
        BlockPos pos(current);
        const AABB* shape = mLevel->getBlockState(pos.below())->getShape(mLevel, pos.below());
        Vec3* target = current->subtract(0.0, 1.0 - shape->mMaxY, 0.0);
        static_cast<MoveControl*>(static_cast<Mob*>(mMob)->getMoveControl())->setWantedPosition(
            target->x, target->y, target->z, mSpeedModifier);
    }
}

void DrownedPathNavigation::updatePath()
{
    if (!mMob->isInWaterOrBubble())
    {
        PathNavigation::updatePath();
        return;
    }
    Vec3* mobPos = getTempMobPos();
    if (mPath == 0 || mPath->isDone())
        return;
    if (mobPos->distanceToSqr(mPath->getPos(mMob->shared_from_this(), mPath->getIndex())) < mMob->mBbWidth * mMob->mBbWidth)
        mPath->next();
    int best = mPath->getIndex() + 6;
    if (best >= mPath->getSize())
        best = mPath->getSize() - 1;
    const double maxDistance = 36.0;
    for (int index = best; mPath->getIndex() < index; --index)
    {
        Vec3* target = mPath->getPos(mMob->shared_from_this(), index);
        if (mobPos->distanceToSqr(target) <= maxDistance && canMoveDirectly(mobPos, target, 0, 0, 0))
        {
            mPath->setIndex(index);
            doStuckDetection(mobPos);
            return;
        }
    }
    doStuckDetection(mobPos);
}

bool DrownedPathNavigation::canMoveDirectly(Vec3* start, Vec3* end, int width, int height, int depth)
{
    if (mMob->isInWaterOrBubble())
    {
        Vec3* target = Vec3::newTemp(end->x, end->y + mMob->mBbHeight * 0.5, end->z);
        HitResult* result = mLevel->clip(start, target, false, true, false, false, 0, false);
        return result == 0 || result->mType == HitResult::MISS;
    }
    return GroundPathNavigation::canMoveDirectly(start, end, width, height, depth);
}

bool DrownedPathNavigation::isStableDestination(const BlockPos& pos)
{
    if (mLevel->isDay() && mLevel->canSeeSky(pos))
        return false;
    if (mMob->isInWaterOrBubble())
        return !mLevel->getBlockState(pos.below())->isSolid();
    return PathNavigation::isStableDestination(pos);
}
