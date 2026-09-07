#include "game/entity/ai/FollowOwnerGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/Player.hpp"

FollowOwnerGoal::FollowOwnerGoal(TamableAnimal* tamable, double speedModifier, float startDistance, float stopDistance)
    : mTamable(tamable), mOwner(), mLevel(tamable->mLevel), mSpeedModifier(speedModifier),
      mNavigation(tamable->getNavigation()), mTimeToRecalcPath(0), mStopDistance(stopDistance),
      mStartDistance(startDistance), mOldWaterCost(0.0f)
{
    setRequiredControlFlags(3);
}

FollowOwnerGoal::~FollowOwnerGoal()
{
}

bool FollowOwnerGoal::canUse()
{
    boost::shared_ptr<Entity> ownerEntity = boost::static_pointer_cast<Entity>(mTamable->getOwner());
    boost::shared_ptr<LivingEntity> owner = boost::dynamic_pointer_cast<LivingEntity>(ownerEntity);
    if (!owner)
        return false;

    if ((owner->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(owner);
        if (player->isSpectator())
            return false;
    }

    if (mTamable->isSitting())
        return false;

    boost::shared_ptr<Entity> entity = boost::static_pointer_cast<Entity>(owner);
    if (mTamable->distanceToSqr(entity) < (double)(mStartDistance * mStartDistance))
        return false;

    mOwner = owner;
    return true;
}

bool FollowOwnerGoal::canContinueToUse()
{
    if (!mOwner.lock())
        return false;
    if (mNavigation->isDone())
        return false;

    boost::shared_ptr<Entity> owner = boost::static_pointer_cast<Entity>(mOwner.lock());
    return mTamable->distanceToSqr(owner) > (double)(mStopDistance * mStopDistance)
        && !mTamable->isSitting();
}

void FollowOwnerGoal::start()
{
    mTimeToRecalcPath = 0;
    mOldWaterCost = mTamable->getPathfindingMalus(BlockPathTypes::WATER);
    mTamable->setPathfindingMalus(BlockPathTypes::WATER, 0.0f);
}

void FollowOwnerGoal::stop()
{
    mOwner.reset();
    mNavigation->stop();
    mTamable->setPathfindingMalus(BlockPathTypes::WATER, mOldWaterCost);
}

void FollowOwnerGoal::tick()
{
    boost::shared_ptr<LivingEntity> owner = mOwner.lock();
    boost::shared_ptr<Entity> ownerEntity = boost::static_pointer_cast<Entity>(owner);
    mTamable->getLookControl()->setLookAt(ownerEntity, 10.0f, (float)mTamable->getMaxHeadXRot());

    if (mTamable->isSitting() || mTamable->isPassenger())
        return;

    --mTimeToRecalcPath;
    if (mTimeToRecalcPath > 0)
        return;

    mTimeToRecalcPath = 10;
    if (mNavigation->moveTo(ownerEntity, mSpeedModifier))
        return;
    if (mTamable->isLeashed() || mTamable->isPassenger())
        return;
    if (mTamable->distanceToSqr(ownerEntity) < 144.0)
        return;

    int x = Mth::floor(owner->mX) - 2;
    int z = Mth::floor(owner->mZ) - 2;
    int y = Mth::floor(owner->mBoundingBox->mMinY);
    int minX = -1;
    int minZ = -1;
    int maxX = -1;
    int maxZ = -1;

    if (owner->isPassenger())
    {
        boost::shared_ptr<Entity> vehicle = owner->getVehicle();
        if (vehicle->mXd <= 0.52)
        {
            minX = 0;
            maxX = 2;
        }
        else
        {
            minX = 2;
            maxX = 4;
        }

        if (vehicle->mZd <= 0.52)
        {
            minZ = 0;
            maxZ = 2;
        }
        else
        {
            minZ = 2;
            maxZ = 4;
        }
    }

    for (int dx = 0; dx <= 4; ++dx)
    {
        for (int dz = 0; dz <= 4; ++dz)
        {
            if (dx >= minX && dz >= minZ && dx <= maxX && dz <= maxZ)
                continue;
            if (!isTeleportFriendlyBlock(x, z, y, dx, dz))
                continue;

            mTamable->moveTo((double)(x + dx) + 0.5, (double)y, (double)(z + dz) + 0.5,
                             mTamable->mYRot, mTamable->mXRot);
            mNavigation->stop();
            return;
        }
    }
}

void FollowOwnerGoal::setLevel(Level* level)
{
    mLevel = level;
}

std::wstring FollowOwnerGoal::getName()
{
    return std::wstring(L"FollowOwner");
}

bool FollowOwnerGoal::isTeleportFriendlyBlock(int x, int z, int y, int dx, int dz)
{
    BlockPos blockPos(x + dx, y - 1, z + dz);
    const BlockState* blockState = mLevel->getBlockState(blockPos);
    if (blockState->getBlockFaceShape(mLevel, blockPos, Direction::DOWN) != BLOCK_FACE_SOLID)
        return false;
    if (!blockState->isValidSpawn(mTamable->shared_from_this()))
        return false;
    return mLevel->isEmptyBlock(blockPos.above()) && mLevel->isEmptyBlock(blockPos.above(2));
}
