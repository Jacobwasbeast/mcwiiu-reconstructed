#include "game/entity/ai/WallClimberNavigation.hpp"

#include <cmath>

#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

WallClimberNavigation::WallClimberNavigation(Mob* mob, Level* level)
    : GroundPathNavigation(mob, level, false),
      mTargetPos(0)
{
    mPathFinder = GroundPathNavigation::createPathFinder();
}

WallClimberNavigation::~WallClimberNavigation()
{
    delete mTargetPos;
}

void WallClimberNavigation::setPosition(BlockPos* pos)
{
    delete mTargetPos;
    mTargetPos = pos;
}

Path* WallClimberNavigation::createPath(const BlockPos& pos)
{
    setPosition(new BlockPos(pos));
    return GroundPathNavigation::createPath(pos);
}

Path* WallClimberNavigation::createPath(boost::shared_ptr<Entity> entity)
{
    setPosition(new BlockPos(entity));
    return GroundPathNavigation::createPath(entity);
}

bool WallClimberNavigation::moveTo(boost::shared_ptr<Entity> entity, double speed)
{
    Path* path = createPath(entity);
    if (path == 0)
    {
        setPosition(new BlockPos(entity));
        mSpeedModifier = speed;
        return true;
    }
    return moveTo(path, speed);
}

void WallClimberNavigation::tick()
{
    if (!isDone())
    {
        PathNavigation::tick();
        return;
    }
    if (mTargetPos == 0)
    {
        return;
    }

    const double widthSquared = (double)(mMob->mBbWidth * mMob->mBbWidth);
    if (widthSquared <= mMob->distanceSqrToBlockPosCenter(*mTargetPos))
    {
        if (mMob->mY <= (double)mTargetPos->y)
        {
            mMob->getMoveControl()->setWantedPosition(
                (double)mTargetPos->x, (double)mTargetPos->y, (double)mTargetPos->z, mSpeedModifier);
            return;
        }

        BlockPos pos(mTargetPos->x, (int)std::floor(mMob->mY), mTargetPos->z);
        if (widthSquared <= mMob->distanceSqrToBlockPosCenter(pos))
        {
            mMob->getMoveControl()->setWantedPosition(
                (double)mTargetPos->x, (double)mTargetPos->y, (double)mTargetPos->z, mSpeedModifier);
            return;
        }
    }
    mTargetPos = 0;
}
