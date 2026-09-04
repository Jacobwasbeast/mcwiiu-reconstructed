#include "game/entity/ai/FindUnderwaterTreasureGoal.hpp"

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/core/Direction.hpp"
#include "game/world/block/BlockPos.hpp"

FindUnderwaterTreasureGoal::FindUnderwaterTreasureGoal(PathfinderMob* mob, float speedModifier, int searchRadius, int maxDistance)
    : Goal(),
      mBribed(false),
      mTreasurePos(NULL),
      mTick(0),
      mStartPos(NULL),
      mDistanceTravelled(0),
      mMaxDistance(maxDistance),
      mStuck(0),
      mDirectionChanges(0),
      mSearchRadius(searchRadius),
      mSpeedModifier(speedModifier),
      mMob((Entity*)mob)
{
}

FindUnderwaterTreasureGoal::~FindUnderwaterTreasureGoal()
{
    delete mTreasurePos;
    delete mStartPos;
}

bool FindUnderwaterTreasureGoal::canUse()
{
    return mBribed && mMob != NULL && mMob->isInWaterOrBubble();
}

bool FindUnderwaterTreasureGoal::canContinueToUse()
{
    return canUse() && mTreasurePos != NULL;
}

bool FindUnderwaterTreasureGoal::canInterrupt()
{
    return false;
}

void FindUnderwaterTreasureGoal::start()
{
    if (mMob == NULL)
        return;

    Vec3* position = mMob->getPos(0.0f);
    BlockPos current(position);
    delete mTreasurePos;
    mTreasurePos = new BlockPos(current);

    delete mStartPos;
    mStartPos = new BlockPos(current);
    mTick = 10;
    mDistanceTravelled = 0;
    mStuck = 0;
    mDirectionChanges = 0;
}

void FindUnderwaterTreasureGoal::tick()
{
    if (mMob == NULL || mTreasurePos == NULL || !mBribed)
        return;

    if (mTick > 0)
    {
        --mTick;
        return;
    }
    mTick = 10;

    if (mMob->distanceToSqr(*mTreasurePos) < 225.0 || mDirectionChanges > 3)
    {
        delete mTreasurePos;
        mTreasurePos = NULL;
        mBribed = false;
        return;
    }

    Vec3* current = mMob->getPos(0.0f);
    int distance = (int)std::abs(mMob->distanceTo(mTreasurePos->x, mTreasurePos->y, mTreasurePos->z));
    if (distance < 1)
    {
        ++mStuck;
        ++mDirectionChanges;
    }
    else
    {
        mStuck = 0;
        mDistanceTravelled += distance;
    }

    if (mDistanceTravelled > mMaxDistance)
    {
        mBribed = false;
        return;
    }

    if (mStuck > 1)
    {
        Direction const* direction = Direction::fromVec3(current);
        if (direction != NULL)
        {
            if (mMob->mRandom != NULL && mMob->mRandom->nextInt(2) == 1)
                direction = direction->getClockWise();
            else
                direction = direction->getCounterClockWise();
            Vec3* target = mMob->getPos(0.0f);
            BlockPos destination(target->x + direction->getStepX() * 10.0,
                                 target->y + direction->getStepY() * 10.0,
                                 target->z + direction->getStepZ() * 10.0);
            delete mTreasurePos;
            mTreasurePos = new BlockPos(destination);
        }
    }
}

void FindUnderwaterTreasureGoal::stop()
{
    delete mTreasurePos;
    mTreasurePos = NULL;
    delete mStartPos;
    mStartPos = NULL;
    mBribed = false;
}

void FindUnderwaterTreasureGoal::setBribed()
{
    mBribed = true;
}

unsigned char FindUnderwaterTreasureGoal::getBribed()
{
    return mBribed;
}

std::wstring FindUnderwaterTreasureGoal::getName()
{
    return std::wstring(L"Find_Treasure");
}
