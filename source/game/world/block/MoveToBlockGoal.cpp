#include "game/world/block/MoveToBlockGoal.hpp"

#include <cmath>

#include "game/core/Node.hpp"
#include "game/core/Path.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

MoveToBlockGoal::MoveToBlockGoal(PathfinderMob* mob, double speedModifier,
                                 int searchRange, int verticalSearchStart,
                                 double maxStayDistance)
    : mMob(mob),
      mNextStartTick(0),
      mTicksStayingStill(0),
      mMaxStayDuration(0),
      mBlockPos(),
      mReachedTarget(false),
      mSearchRange(searchRange),
      mVerticalSearchStart(verticalSearchStart),
      mSpeedModifierSquared(maxStayDistance * maxStayDistance),
      mSpeedModifier(speedModifier)
{
    setRequiredControlFlags(5);
}

bool MoveToBlockGoal::findNearestBlock()
{
    BlockPos origin(mMob);
    BlockPos minPos = origin.offset(-mSearchRange, -(mVerticalSearchStart + 1),
                                     -mSearchRange);
    BlockPos maxPos = origin.offset(mSearchRange, mVerticalSearchStart,
                                     mSearchRange);

    if (!isValidTargetArea(mMob->mLevel, minPos, maxPos) ||
        mVerticalSearchStart < 0)
    {
        return false;
    }

    for (int yIndex = 0; yIndex <= mVerticalSearchStart * 2 + 1; ++yIndex)
    {
        int y = (yIndex & 1) == 0 ? -(yIndex / 2 + 1) : yIndex / 2;
        for (int radius = 0; radius <= mSearchRange; ++radius)
        {
            for (int x = -radius; x <= radius; ++x)
            {
                int z = radius;
                BlockPos candidate = origin.offset(x, y - 1, z);
                if (mMob->mLevel->isEmptyBlock(candidate) &&
                    isValidTarget(mMob->mLevel, candidate))
                {
                    mBlockPos = candidate;
                    return true;
                }
                if (radius != 0)
                {
                    candidate = origin.offset(x, y - 1, -z);
                    if (mMob->mLevel->isEmptyBlock(candidate) &&
                        isValidTarget(mMob->mLevel, candidate))
                    {
                        mBlockPos = candidate;
                        return true;
                    }
                }
            }
            for (int z = radius - 1; z > -radius; --z)
            {
                int x = radius;
                BlockPos candidate = origin.offset(x, y - 1, z);
                if (mMob->mLevel->isEmptyBlock(candidate) &&
                    isValidTarget(mMob->mLevel, candidate))
                {
                    mBlockPos = candidate;
                    return true;
                }
                if (radius != 0)
                {
                    candidate = origin.offset(-x, y - 1, z);
                    if (mMob->mLevel->isEmptyBlock(candidate) &&
                        isValidTarget(mMob->mLevel, candidate))
                    {
                        mBlockPos = candidate;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool MoveToBlockGoal::canUse()
{
    if (mNextStartTick > 0)
    {
        --mNextStartTick;
        return false;
    }

    mNextStartTick = GetNextStartTick(mMob);
    return findNearestBlock();
}

bool MoveToBlockGoal::canContinueToUse()
{
    return mTicksStayingStill >= -mMaxStayDuration &&
           mTicksStayingStill < 1200 && canReach(mBlockPos);
}

void MoveToBlockGoal::start()
{
    mMob->getNavigation()->moveTo((double)mBlockPos.x + 0.5,
                                  (double)mBlockPos.y + 1.0,
                                  (double)mBlockPos.z + 0.5,
                                  mSpeedModifier);
    mTicksStayingStill = 0;
    mMaxStayDuration = 1200 +
        mMob->getRandom()->nextInt(1200 + mMob->getRandom()->nextInt(1200));
}

int MoveToBlockGoal::yOff()
{
    return 1;
}

void MoveToBlockGoal::tick()
{
    BlockPos above = mBlockPos.above();
    if (mMob->distanceSqrToBlockPosCenter(above) <= mSpeedModifierSquared)
    {
        mReachedTarget = true;
        --mTicksStayingStill;
    }
    else
    {
        ++mTicksStayingStill;
        mReachedTarget = false;
        if (mTicksStayingStill % 40 == 0)
        {
            mMob->getNavigation()->moveTo((double)mBlockPos.x + 0.5,
                                          (double)mBlockPos.y + (double)yOff(),
                                          (double)mBlockPos.z + 0.5,
                                          mSpeedModifier);
        }
    }
}

bool MoveToBlockGoal::isReachedTarget()
{
    return mReachedTarget;
}

int MoveToBlockGoal::GetNextStartTick(PathfinderMob* mob)
{
    return mob->getRandom()->nextInt(200) + 200;
}

bool MoveToBlockGoal::canReach(const BlockPos& pos)
{
    Vec3* target = Vec3::newTemp((double)pos.x, (double)pos.y + 1.0,
                                 (double)pos.z);
    Path* path = mMob->getNavigation()->createPath(BlockPos(target));
    if (path == NULL || path->last() == NULL)
    {
        return false;
    }

    Node* last = path->last();
    double halfWidth = (double)mMob->mBbWidth * 0.5;
    double x = target->x + 0.5 - halfWidth;
    double z = target->z + 0.5 - halfWidth;
    int targetX = (int)std::floor(x);
    int targetZ = (int)std::floor(z);
    bool result = (last->x - targetX) * (last->x - targetX) +
                  (last->z - targetZ) * (last->z - targetZ) < 2 &&
                  (double)last->y == target->y;
    delete path;
    return result;
}

std::wstring MoveToBlockGoal::getName()
{
    return std::wstring(L"MoveToBlock");
}

bool MoveToBlockGoal::isValidTargetArea(Level*, const BlockPos&, const BlockPos&)
{
    return true;
}
