#include "game/entity/ai/WaterBoundPathNavigation.hpp"

#include "game/world/block/Block.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Path.hpp"
#include "game/core/PathFinder.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"
#include "game/util/Vec3.hpp"

#include <boost/unordered_set.hpp>

WaterBoundPathNavigation::WaterBoundPathNavigation(Mob* mob, Level* level, bool createPathFinder,
                                                   bool allowBreaching)
    : PathNavigation(mob, level), mNodeEvaluator(allowBreaching)
{
    if (createPathFinder)
        mPathFinder = this->createPathFinder();
}

WaterBoundPathNavigation::~WaterBoundPathNavigation() = default;

PathFinder* WaterBoundPathNavigation::createPathFinder()
{
    return new PathFinder(&mNodeEvaluator);
}

bool WaterBoundPathNavigation::canUpdatePath()
{
    return isInLiquid();
}

Vec3* WaterBoundPathNavigation::getTempMobPos()
{
    return Vec3::newTemp(mMob->mX, mMob->mY + (double)mMob->mBbHeight * 0.5, mMob->mZ);
}

void WaterBoundPathNavigation::updatePath()
{
    boost::shared_ptr<Entity> entity = mMob->shared_from_this();
    Vec3* currentPos = mPath->currentPos(entity);
    Vec3* mobPos = getTempMobPos();
    double speed = 0.2;
    float width = mMob->mBbWidth;
    float distance = width <= 0.75f ? 0.75f - width * 0.5f : width * 0.5f;
    double motionLength = Mth::sqrt(mMob->mXd * mMob->mXd + mMob->mYd * mMob->mYd + mMob->mZd * mMob->mZd);
    if (Mth::abs(mMob->mXd) > speed || Mth::abs(mMob->mZd) > speed)
        distance = distance * (float)(motionLength * 0.4);

    if (Mth::abs(mMob->mX - (currentPos->x + 0.5)) < distance &&
        Mth::abs(mMob->mZ - (currentPos->z + 0.5)) < distance &&
        Mth::abs(mMob->mY - currentPos->y) < distance + distance)
        mPath->next();

    int index = mPath->getIndex();
    int size = mPath->getSize();
    int best = index + 6;
    if (size - 1 < best)
        best = size - 1;

    index = mPath->getIndex();
    while (index < best)
    {
        Vec3* position = mPath->getPos(entity, best);
        if (position->distanceToSqr(mobPos) <= 36.0 &&
            canMoveDirectly(mobPos, position, 0, 0, 0))
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

bool WaterBoundPathNavigation::canMoveDirectly(Vec3* a, Vec3* b, int x, int y, int z)
{
    Vec3* target = Vec3::newTemp(b->x, b->y + (double)mMob->mBbHeight * 0.5, b->z);
    HitResult* result = mLevel->clip(a, target, false, true, false, false, NULL, false);
    bool clear = result == NULL || result->mType == HitResult::MISS;
    delete result;
    return clear;
}

NodeEvaluator* WaterBoundPathNavigation::getNodeEvaluator()
{
    return &mNodeEvaluator;
}

bool WaterBoundPathNavigation::isStableDestination(const BlockPos& pos)
{
    return !mLevel->getBlock(pos)->isPathfindable(mLevel, pos);
}
