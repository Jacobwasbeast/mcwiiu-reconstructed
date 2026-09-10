#include "game/entity/ai/PathNavigation.hpp"
#include "game/core/Path.hpp"
#include "game/core/PathFinder.hpp"
#include "game/core/Node.hpp"
#include "game/core/Mth.hpp"
#include "game/core/System.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include <cmath>

PathNavigation::PathNavigation(Mob* mob, Level* level)
    : mPath(0), mMob(mob), mLevel(level), mTick(0), mSpeedModifier(1.0), mTargetPos(0), mMaxDistance(0), mLastStuckCheck(0), mLastStuckCheckPos(Vec3::newPermanent(0.0, 0.0, 0.0)), mLastTargetPos(Vec3::newPermanent(0.0, 0.0, 0.0)), mStuckTime(0), mLastStuckCheckTime(0), mLastStuckCheckTimeHigh(0), mTimeoutCachedNode(0), mTimeoutLimit(1.0), mIsRiverFollowing(false), mMaxDistanceToWaypoint(0.5f), mHasDelayedRecomputation(false), mLastRecomputeTime(0), mTargetPath(0), mPathFinder(0)
{
}

PathNavigation::~PathNavigation()
{
    delete mPath;
    delete mLastStuckCheckPos;
    delete mLastTargetPos;
    delete mPathFinder;
    delete mTargetPos;
}

bool PathNavigation::shouldAvoidSun() const { return false; }

bool PathNavigation::isDone() { return mPath == 0 || mPath->isDone(); }

void PathNavigation::stop()
{
    delete mPath;
    mPath = 0;
    delete mTargetPos;
    mTargetPos = 0;
}

PathFinder* PathNavigation::getPathFinder() { return mPathFinder; }
void PathNavigation::setSpeedModifier(double speed) { mSpeedModifier = speed; }
double PathNavigation::getMaxDist() { return mMob->mBbWidth; }
bool PathNavigation::hasDelayedRecomputation() { return mHasDelayedRecomputation; }

void PathNavigation::recomputePath()
{
    if (mLevel->getGameTime() - mLastRecomputeTime < 21) mHasDelayedRecomputation = true;
    else if (mTargetPos != 0)
    {
        delete mPath;
        mPath = createPath(*mTargetPos);
        mHasDelayedRecomputation = false;
        mLastRecomputeTime = mLevel->getGameTime();
    }
}

Path* PathNavigation::createPath(double x, double y, double z)
{
    BlockPos pos(x, y, z);
    return createPath(pos);
}

Path* PathNavigation::createPath(const BlockPos& pos)
{
    if (!canUpdatePath()) return 0;
    if (mPath != 0 && !mPath->isDone() && mTargetPos != 0 && pos.equals(*mTargetPos)) return mPath->clone();
    delete mTargetPos;
    mTargetPos = new BlockPos(pos);
    return mPathFinder->findPath(mLevel, mMob, pos, (float)getMaxDist());
}

Path* PathNavigation::createPath(boost::shared_ptr<Entity> entity)
{
    if (!canUpdatePath()) return 0;
    BlockPos pos(entity);
    if (mPath != 0 && !mPath->isDone() && mTargetPos != 0 && pos.equals(*mTargetPos)) return mPath->clone();
    delete mTargetPos;
    mTargetPos = new BlockPos(pos);
    return mPathFinder->findPath(mLevel, mMob, entity.get(), (float)getMaxDist());
}

Path* PathNavigation::createDirectPath(const BlockPos& pos)
{
    if (!canUpdatePath()) return 0;
    delete mTargetPos;
    mTargetPos = new BlockPos(pos);
    return mPathFinder->findDirectPath(mLevel, mMob, pos, (float)getMaxDist());
}

bool PathNavigation::moveTo(double x, double y, double z, double speed)
{
    Path* path = createPath(x, y, z);
    return path != 0 && moveTo(path, speed);
}

bool PathNavigation::moveTo(boost::shared_ptr<Entity> entity, double speed)
{
    Path* path = createPath(entity);
    return path != 0 && moveTo(path, speed);
}

bool PathNavigation::moveTo(Path* path, double speed)
{
    if (path == 0) { stop(); return false; }
    if (mPath != path) { delete mPath; mPath = path; updatePath(); }
    if (mPath->getSize() <= 0) return false;
    mSpeedModifier = speed;
    mLastTargetPos->set(mPath->currentPos());
    return true;
}

Path* PathNavigation::getPath() { return mPath; }

void PathNavigation::tick()
{
    ++mTick;
    if (mHasDelayedRecomputation) recomputePath();
    if (isDone()) return;
    if (!canUpdatePath())
    {
        Vec3* p = getTempMobPos();
        Vec3* next = mPath->getPos(boost::shared_ptr<Entity>(), mPath->getIndex());
        if (next->y < p->y && !mMob->mOnGround) return;
        if (std::floor(p->x) == std::floor(next->x) && std::floor(p->z) == std::floor(next->z)) mPath->setIndex(mPath->getIndex() + 1);
    }
    else updatePath();
}

void PathNavigation::doStuckDetection(Vec3* pos)
{
    if (mTick - mLastStuckCheck > 100)
    {
        if (pos->distanceToSqr(mLastStuckCheckPos) < 2.25) stop();
        mLastStuckCheck = mTick;
        mLastStuckCheckPos->set(pos);
    }
    if (mPath != 0 && !mPath->isDone())
    {
        Vec3* current = mPath->currentPos();
        if (!current->equals(mLastTargetPos))
        {
            mLastTargetPos->set(current);
            double distance = pos->distanceTo(mLastTargetPos);
            double speed = mMob->getSpeed();
            mTimeoutLimit = speed <= 0.0 ? 0.0 : distance / speed * 1000.0;
        }
        else if (System::processTimeInMilliSecs() - mLastStuckCheckTime > (unsigned int)mTimeoutLimit * 3) stop();
        mLastStuckCheckTime = System::processTimeInMilliSecs();
    }
}

void PathNavigation::updatePath()
{
    if (mPath == 0 || mPath->isDone()) return;
    Vec3* mobPos = getTempMobPos();
    int index = mPath->getIndex();
    int size = mPath->getSize();
    int best = index;
    int i;
    for (i = index; i < size; ++i)
    {
        Node* node = mPath->get(i);
        Vec3 target((double)node->x + 0.5, (double)node->y, (double)node->z + 0.5);
        if (!canMoveDirectly(mobPos, &target, (int)std::ceil(mMob->mBbWidth), (int)std::ceil(mMob->mBbHeight), (int)std::ceil(mMob->mBbWidth))) break;
        best = i;
    }
    if (best > index) mPath->setIndex(best);
    doStuckDetection(mobPos);
}
bool PathNavigation::isInLiquid() { return mMob->isInWaterOrBubble() || mMob->isInLava(); }
void PathNavigation::trimPath()
{
    if (mPath == 0) return;
    int size = mPath->getSize();
    int i;
    for (i = 0; i < size; ++i)
    {
        Node* node = mPath->get(i);
        Node* next = i + 1 < size ? mPath->get(i + 1) : 0;
        if (mLevel->getBlockId(node->x, node->y, node->z) == 0x76)
        {
            mPath->set(i, node->cloneMove(node->x, node->y + 1, node->z));
            if (next != 0 && next->y <= node->y) mPath->set(i + 1, next->cloneMove(next->x, node->y + 1, next->z));
        }
    }
}
bool PathNavigation::isStableDestination(const BlockPos& pos) { return mLevel->getBlockState(pos.below())->isSolid(); }
void PathNavigation::setLevel(Level* level) { mLevel = level; }
bool PathNavigation::isRiverFollowing() { return mIsRiverFollowing; }
void PathNavigation::setIsRiverFollowing(bool value) { mIsRiverFollowing = value; }
void PathNavigation::debugPathfinding() { }
