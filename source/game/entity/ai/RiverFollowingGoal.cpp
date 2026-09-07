#include "game/entity/ai/RiverFollowingGoal.hpp"

#include <cmath>

#include "game/core/HitResult.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

RiverFollowingGoal::RiverFollowingGoal(Mob* mob, float distance, float speed)
    : Goal()
{
    mMob = mob;
    mDistance = distance;
    mSpeed = speed;
    mSteerDirection = NULL;
    setRequiredControlFlags(3);
}

RiverFollowingGoal::~RiverFollowingGoal()
{
}

bool RiverFollowingGoal::canUse()
{
    PathNavigation* navigation = mMob->getNavigation();
    if (navigation == NULL || mMob->followsLeader())
    {
        return false;
    }

    BlockPos position(mMob->getPos(1.0f));
    if (mMob->mLevel->getBiome(position)->getBiomeType() == Biome::eBiomeType_Ocean)
    {
        return false;
    }

    return mMob->isInWater();
}

void RiverFollowingGoal::start()
{
    PathNavigation* navigation = mMob->getNavigation();
    if (navigation != NULL)
    {
        navigation->setIsRiverFollowing(true);
        navigation->stop();
    }
}

void RiverFollowingGoal::stop()
{
    PathNavigation* navigation = mMob->getNavigation();
    if (navigation != NULL)
    {
        navigation->setIsRiverFollowing(false);
    }
    mMob->setJumping(false);
}

void RiverFollowingGoal::determineSteerDirection()
{
    Vec3* position = mMob->getPos(1.0f);
    Vec3* direction = mMob->getLookAngle();
    mSteerDirection = Vec3::newTemp(direction);

    Vec3* left = Vec3::newTemp(direction)->yRot(15.0f);
    Vec3* right = Vec3::newTemp(direction)->yRot(-15.0f);
    HitResult* forwardHit = mMob->mLevel->clip(position, Vec3::newTemp(direction)->multiply(mDistance)->add(position), false, true, false, false, NULL, false);
    HitResult* leftHit = mMob->mLevel->clip(position, left->multiply(mDistance)->add(position), false, true, false, false, NULL, false);
    HitResult* rightHit = mMob->mLevel->clip(position, right->multiply(mDistance)->add(position), false, true, false, false, NULL, false);

    if (forwardHit != NULL && forwardHit->mType != HitResult::MISS)
    {
        double bestAngle = 3.402823466e+38;
        Vec3* best = mSteerDirection;
        Vec3* candidates[3] = { direction, left, right };
        HitResult* hits[3] = { forwardHit, leftHit, rightHit };
        int i;
        for (i = 0; i < 3; ++i)
        {
            bool blocked = hits[i] != NULL && hits[i]->mType != HitResult::MISS;
            if (blocked)
            {
                const BlockState* state = mMob->mLevel->getBlockState(hits[i]->getBlockPos());
                blocked = state != NULL && state->isSolid();
            }
            if (!blocked)
            {
                double angle = std::acos(direction->dot(candidates[i]));
                if (angle < bestAngle)
                {
                    bestAngle = angle;
                    best = candidates[i];
                }
            }
        }
        mSteerDirection = Vec3::newTemp(best);
    }

    Vec3* turnLeft = Vec3::newTemp(direction)->yRot(45.0f);
    Vec3* turnRight = Vec3::newTemp(direction)->yRot(-45.0f);
    HitResult* turnLeftHit = mMob->mLevel->clip(position, turnLeft->multiply(mDistance)->add(position), false, true, false, false, NULL, false);
    HitResult* turnRightHit = mMob->mLevel->clip(position, turnRight->multiply(mDistance)->add(position), false, true, false, false, NULL, false);
    if (turnLeftHit != NULL && turnLeftHit->mType == HitResult::BLOCK)
    {
        mSteerDirection->add(turnLeft->normalize()->multiply(1.2));
    }
    if (turnRightHit != NULL && turnRightHit->mType == HitResult::BLOCK)
    {
        mSteerDirection->add(turnRight->normalize()->multiply(1.2));
    }

    BlockPos positionBlock(mMob->getPos(1.0f));
    if (!mMob->followsLeader() && mMob->mLevel->getBiome(positionBlock)->getBiomeType() == Biome::eBiomeType_Ocean)
    {
        mSteerDirection->add(0.0, -0.25, 0.0);
    }
}

void RiverFollowingGoal::tick()
{
    determineSteerDirection();
    if (!mMob->followsLeader() || mMob->isLeader())
    {
        mSteerDirection->normalize()->multiply(mSpeed);
        float rotation = (float)std::atan2(mSteerDirection->x, mSteerDirection->z);
        mMob->mYRot = Mth::clampRotateDegrees(mMob->mYRot, -rotation * 57.2957763671875f, 45.0f);
        mMob->moveRelative((float)mSteerDirection->x, (float)mSteerDirection->y, (float)mSteerDirection->z, 1.0f);
        mMob->setJumping(true);
    }
}

bool RiverFollowingGoal::canContinueToUse()
{
    if (mMob->followsLeader())
    {
        return false;
    }
    BlockPos position(mMob->getPos(1.0f));
    return mMob->mLevel->getBiome(position)->getBiomeType() != Biome::eBiomeType_Ocean;
}

std::wstring RiverFollowingGoal::getName()
{
    return L"RiverFollowing";
}
