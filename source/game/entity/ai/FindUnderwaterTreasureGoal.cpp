#include "game/entity/ai/FindUnderwaterTreasureGoal.hpp"

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/core/Direction.hpp"
#include "game/world/Level.hpp"
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
    Vec3* position = mMob->getPos(0.0f);
    BlockPos current(position);
    delete mTreasurePos;
    mTreasurePos = new BlockPos(current);

    mTick = 10;
    mDistanceTravelled = 0;
    delete mStartPos;
    mStartPos = new BlockPos(mMob->mX, mMob->mY, mMob->mZ);
    mStuck = 0;
    mDirectionChanges = 0;

    if (mTreasurePos != NULL)
    {
        if (!static_cast<Mob*>(mMob)->getNavigation()->isDone())
            static_cast<Mob*>(mMob)->getNavigation()->stop();
    }
}

void FindUnderwaterTreasureGoal::tick()
{
    if (mTreasurePos == NULL || !mBribed)
        return;

    if (--mTick > 0)
        return;
    mTick = 10;

    if (mMob->distanceToSqr(*mTreasurePos) < 225.0 || mDirectionChanges > 3)
    {
        delete mTreasurePos;
        mTreasurePos = NULL;
        mBribed = false;
        return;
    }

    int distance = (int)std::abs(mMob->distanceTo((double)mStartPos->x, (double)mStartPos->y, (double)mStartPos->z));
    if (distance < 1)
    {
        ++mStuck;
        ++mDirectionChanges;
        mDistanceTravelled += distance;
    }
    else
    {
        mStuck = 0;
        mDistanceTravelled += distance;
    }

    delete mStartPos;
    mStartPos = new BlockPos(mMob->getPos(0.0f));

    if (mMaxDistance < mDistanceTravelled)
    {
        mBribed = false;
        return;
    }

    BlockPos topSolid = mMob->mLevel->getTopSolidBlockPos(*mTreasurePos);
    Vec3* current = mMob->getPos(0.0f);
    Vec3* direction = Vec3::newTemp((double)topSolid.x - current->x,
                                     (double)topSolid.y - current->y,
                                     (double)topSolid.z - current->z);

    double targetX = topSolid.x;
    double targetY = topSolid.y;
    double targetZ = topSolid.z;

    if (direction->lengthSqr() > 16.0)
    {
        if (mStuck > 1)
        {
            Direction const* facing = Direction::fromVec3(direction);
            Random* random = static_cast<Mob*>(mMob)->getRandom();
            if (facing == Direction::UP || facing == Direction::DOWN)
            {
                facing = facing->getOpposite();
            }
            else if (random->nextInt(2) == 1)
            {
                facing = facing->getClockWise();
            }
            else
            {
                facing = facing->getCounterClockWise();
            }
            BlockPos destination(current->x + facing->getStepX() * 10.0,
                                 current->y + facing->getStepY() * 10.0,
                                 current->z + facing->getStepZ() * 10.0);
            static_cast<Mob*>(mMob)->getNavigation()->moveTo(destination.x, destination.y, destination.z,
                                                              (double)(mSpeedModifier + 1.0f));
            return;
        }

        direction->normalize();
        direction->multiply(16.0 * 0.5);
        current->add(direction);
        BlockPos partial(current);
        targetX = partial.x;
        targetY = partial.y;
        targetZ = partial.z;
    }

    static_cast<Mob*>(mMob)->getNavigation()->moveTo(targetX, targetY, targetZ, (double)(mSpeedModifier + 1.0f));
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
    return std::wstring(L"Find Treasure");
}
