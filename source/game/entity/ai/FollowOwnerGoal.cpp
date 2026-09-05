#include "game/entity/ai/FollowOwnerGoal.hpp"

#include "game/entity/ai/PathNavigation.hpp"
#include "game/core/Direction.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/OwnableEntity.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

FollowOwnerGoal::FollowOwnerGoal(TamableAnimal* tamable, double speedModifier, float startDistance, float stopDistance)
    : mTamable(tamable), mOwner(), mLevel(NULL), mSpeedModifier(speedModifier), mNavigation(NULL),
      mTimeToRecalcPath(0), mStartDistance(startDistance), mStopDistance(stopDistance), mSpeed(0.0f)
{
    setRequiredControlFlags(3);
}

FollowOwnerGoal::~FollowOwnerGoal()
{
}

bool FollowOwnerGoal::canUse()
{
    if (mTamable != NULL && !mOwner)
        mOwner = boost::static_pointer_cast<Entity>(((OwnableEntity*)mTamable)->getOwner());
    if (!mOwner || mTamable == NULL)
        return false;
    return ((Entity*)mTamable)->distanceToSqr(mOwner) >= (double)(mStartDistance * mStartDistance);
}

bool FollowOwnerGoal::canContinueToUse()
{
    if (!mOwner || mNavigation == NULL || mNavigation->isDone())
        return false;
    return mTamable != NULL
        && ((Entity*)mTamable)->distanceToSqr(mOwner) > (double)(mStopDistance * mStopDistance);
}

void FollowOwnerGoal::start()
{
    mTimeToRecalcPath = 0;
    if (mNavigation != NULL && mOwner)
        mNavigation->moveTo(mOwner, mSpeedModifier);
}

void FollowOwnerGoal::stop()
{
    mOwner.reset();
    if (mNavigation != NULL)
        mNavigation->stop();
}

void FollowOwnerGoal::tick()
{
    if (mTamable == NULL || !mOwner)
        return;
    --mTimeToRecalcPath;
    if (mTimeToRecalcPath > 0)
        return;
    mTimeToRecalcPath = 10;
    if (mNavigation != NULL)
        mNavigation->moveTo(mOwner, mSpeedModifier);
}

bool FollowOwnerGoal::isTeleportFriendlyBlock(int x, int z, int y, int dx, int dz)
{
    BlockPos blockPos(x + dx, y - 1, z + dz);
    const BlockState* blockState = mLevel->getBlockState(blockPos);
    if (blockState->getBlockFaceShape(mLevel, blockPos, Direction::DOWN) != BLOCK_FACE_SOLID)
        return false;
    if (!blockState->isValidSpawn(((Entity*)mTamable)->shared_from_this()))
        return false;
    return mLevel->isEmptyBlock(blockPos.above()) && mLevel->isEmptyBlock(blockPos.above(2));
}

void FollowOwnerGoal::setLevel(Level* level)
{
    mLevel = level;
}

std::wstring FollowOwnerGoal::getName()
{
    return std::wstring(L"FollowOwner");
}
