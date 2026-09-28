#include "game/entity/ai/PathNavigation.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/Path.hpp"
#include "game/core/PathFinder.hpp"
#include "game/core/Node.hpp"
#include "game/core/Mth.hpp"
#include "game/core/System.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/Region.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/util/AABB.hpp"
#include <cmath>

PathNavigation::PathNavigation(Mob* mob, Level* level)
    : mPath(0), mMob(mob), mLevel(level), mSpeedModifier(1.0), mFollowRange(mob->getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)), mTick(0), mLastStuckCheck(0), mLastStuckCheckPos(Vec3::newPermanent(0.0, 0.0, 0.0)), mLastTargetPos(Vec3::newPermanent(0.0, 0.0, 0.0)), mStuckTime(0), mLastStuckCheckTime(0), mTimeoutLimit(1.0), mIsRiverFollowing(false), mMaxDistanceToWaypoint(0.5f), mHasDelayedRecomputation(false), mLastRecomputeTime(0), mTargetPos(0), mPathFinder(0)
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
double PathNavigation::getMaxDist()
{
    return (double)(float)mFollowRange->getValue();
}
bool PathNavigation::hasDelayedRecomputation() { return mHasDelayedRecomputation; }

void PathNavigation::recomputePath()
{
    if (mLevel->getGameTime() - mLastRecomputeTime < 21) mHasDelayedRecomputation = true;
    else if (mTargetPos != 0)
    {
        delete mPath;
        mPath = 0;
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
    double maxDist = getMaxDist();
    BlockPos mobPos(mMob);
    int radius = (int)(maxDist + 8.0);
    BlockPos min = mobPos.offset(-radius, -radius, -radius);
    BlockPos max = mobPos.offset(radius, radius, radius);
    Region region(mLevel, min, max, 0);
    return mPathFinder->findPath(&region, mMob, *mTargetPos, (float)maxDist);
}

Path* PathNavigation::createPath(boost::shared_ptr<Entity> entity)
{
    if (!canUpdatePath()) return 0;
    BlockPos pos(entity);
    if (mPath != 0 && !mPath->isDone() && mTargetPos != 0 && pos.equals(*mTargetPos)) return mPath->clone();
    delete mTargetPos;
    mTargetPos = new BlockPos(pos);
    double maxDist = getMaxDist();
    BlockPos mobPos(mMob);
    BlockPos center = mobPos.above();
    int radius = (int)(maxDist + 16.0);
    BlockPos min = center.offset(-radius, -radius, -radius);
    BlockPos max = center.offset(radius, radius, radius);
    Region region(mLevel, min, max, 0);
    return mPathFinder->findPath(&region, mMob, entity.get(), (float)maxDist);
}

Path* PathNavigation::createDirectPath(const BlockPos& pos)
{
    if (!canUpdatePath()) return 0;
    if (mPath != 0 && !mPath->isDone() && pos.equals(mTargetPos)) return mPath->clone();
    delete mTargetPos;
    mTargetPos = new BlockPos(pos);
    double maxDist = getMaxDist();
    BlockPos mobPos(mMob);
    int radius = (int)(maxDist + 8.0);
    BlockPos min = mobPos.offset(-radius, -radius, -radius);
    BlockPos max = mobPos.offset(radius, radius, radius);
    Region region(mLevel, min, max, 0);
    return mPathFinder->findDirectPath(&region, mMob, *mTargetPos, (float)maxDist);
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
    if ((mPath != 0 && mPath != path && mPath->isDone()) || !path->sameAs(mPath))
    {
        delete mPath;
        mPath = path;
    }
    else if (path != mPath)
    {
        delete path;
    }
    trimPath();
    if (mPath->getSize() <= 0) return false;
    mSpeedModifier = speed;
    mLastStuckCheck = mTick;
    mLastTargetPos->set(mPath->currentPos());
    return true;
}

Path* PathNavigation::getPath() { return mPath; }

void PathNavigation::tick()
{
    ++mTick;
    if (mHasDelayedRecomputation) recomputePath();
    if (isDone()) return;
    if (canUpdatePath())
    {
        PIXBeginNamedEvent(0.0f, "updatePath");
        updatePath();
        PIXEndNamedEvent();
    }
    else
    {
        if (mPath != 0 && mPath->getIndex() < mPath->getSize())
        {
            Vec3* p = getTempMobPos();
            Vec3* next = mPath->getPos(mMob->shared_from_this(), mPath->getIndex());
            if (next->y < p->y && !mMob->mOnGround)
            {
                if (std::floor(p->x) == std::floor(next->x) && std::floor(p->z) == std::floor(next->z)) mPath->setIndex(mPath->getIndex() + 1);
            }
        }
    }
    if (!isDone())
    {
        Vec3* currentPos = mPath->currentPos(mMob->shared_from_this());
        if (currentPos != 0)
        {
            BlockPos below = BlockPos(currentPos).below();
            const BlockState* blockState = mLevel->getBlockState(below);
            const AABB* outline = blockState->getOutlineAABB(mLevel, below);
            Vec3* adjusted = currentPos->subtract(0.0, 1.0 - outline->mMaxY, 0.0);
            mMob->getMoveControl()->setWantedPosition(adjusted->x, adjusted->y, adjusted->z, mSpeedModifier);
        }
    }
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
            mTimeoutLimit = speed <= 0.0 ? 1.0 : distance / speed * 1000.0;
        }
        else
        {
            mStuckTime += System::processTimeInMilliSecs() - mLastStuckCheckTime;
        }
        if (mTimeoutLimit > 1.0 && (double)mStuckTime > mTimeoutLimit * 3.0)
        {
            mLastTargetPos->set(0.0, 0.0, 0.0);
            mStuckTime = 0;
            mTimeoutLimit = 1.0;
            stop();
        }
        mLastStuckCheckTime = System::processTimeInMilliSecs();
    }
}

void PathNavigation::updatePath()
{
    Mob* pathOwner = mMob;
    if (Entity::instanceof(mMob->getVehicle(), EntityType::MOB)) pathOwner = static_cast<Mob*>(mMob->getVehicle().get());
    PathNavigation* navigation = pathOwner->getNavigation();
    Vec3* mobPos;
    PathNavigation* mover;
    if (navigation == 0)
    {
        mobPos = getTempMobPos();
        mover = this;
    }
    else
    {
        mobPos = navigation->getTempMobPos();
        mover = navigation;
    }
    int size = mPath->getSize();
    int stopIndex = size;
    if (!pathOwner->canFly() && mPath != 0 && mPath->getIndex() < size)
    {
        int floorY = Mth::floor(mobPos->y);
        int i;
        for (i = mPath->getIndex(); i < size; ++i)
        {
            Node* node = mPath->get(i);
            if (node->y != floorY) { stopIndex = i; break; }
        }
    }
    float halfWidth = pathOwner->mBbWidth;
    mMaxDistanceToWaypoint = halfWidth <= 0.75f ? 0.75f - halfWidth * 0.5f : halfWidth * 0.5f;
    Vec3* currentPos = mPath->currentPos();
    if ((double)Mth::abs((float)(pathOwner->mX - (currentPos->x + 0.5))) < (double)mMaxDistanceToWaypoint &&
        (double)Mth::abs((float)(pathOwner->mZ - (currentPos->z + 0.5))) < (double)mMaxDistanceToWaypoint &&
        Mth::abs(pathOwner->mY - currentPos->y) < 1.0)
    {
        mPath->setIndex(mPath->getIndex() + 1);
    }
    int ceilWidth = (int)std::ceil(pathOwner->mBbWidth);
    int ceilHeight = (int)std::ceil(pathOwner->mBbHeight);
    int index = stopIndex - 1;
    while (mPath->getIndex() <= index)
    {
        if (mover->canMoveDirectly(mobPos, mPath->getPos(pathOwner->shared_from_this(), index), ceilWidth, ceilHeight, ceilWidth))
        {
            mPath->setIndex(index);
            break;
        }
        --index;
    }
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
        if (mLevel->getBlockId(node->x, node->y, node->z) == BlockId::CAULDRON)
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
