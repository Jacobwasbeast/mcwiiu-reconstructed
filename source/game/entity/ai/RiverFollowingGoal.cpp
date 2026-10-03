#include "game/entity/ai/RiverFollowingGoal.hpp"

#include <cmath>

#include "game/core/HitResult.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

RiverFollowingGoal::RiverFollowingGoal(Mob* mob, float distance, float speed)
    : Goal()
{
    mSpeed = speed;
    mMob = mob;
    mDistance = distance;
    setRequiredControlFlags(3);
}

RiverFollowingGoal::~RiverFollowingGoal()
{
}

bool RiverFollowingGoal::canUse()
{
    if (mMob->getNavigation() == NULL || mMob->followsLeader())
        return false;

    BlockPos position(mMob->getPos(0.0f));
    if (mMob->mLevel->getBiome(position)->getBiomeType() == Biome::eBiomeType_Ocean)
        return false;

    return mMob->isInWater();
}

void RiverFollowingGoal::start()
{
    if (mMob->getNavigation() != NULL)
    {
        mMob->getNavigation()->setIsRiverFollowing(true);
        mMob->getNavigation()->stop();
    }
}

void RiverFollowingGoal::stop()
{
    if (mMob->getNavigation() != NULL)
        mMob->getNavigation()->setIsRiverFollowing(false);

    mMob->setMoving(false);
}

void RiverFollowingGoal::determineSteerDirection()
{
    Vec3* position = mMob->getPos(0.0f);
    Vec3* lookAngle = mMob->getLookAngle();
    Level* level = mMob->mLevel;
    mSteerDirection = lookAngle;

    Vec3* left = Vec3::newTemp(lookAngle->x, 0.0, lookAngle->z)->yRot(15.0f);
    Vec3* right = Vec3::newTemp(lookAngle->x, 0.0, lookAngle->z)->yRot(-15.0f);

    HitResult* forwardHit = level->clip(position,
        lookAngle->multiply(mDistance)->add(position), false, true, true, false, NULL, false);
    HitResult* leftHit = level->clip(position,
        left->multiply(mDistance - 1.5f)->add(position), false, true, true, false, NULL, false);
    HitResult* rightHit = level->clip(position,
        right->multiply(mDistance - 1.5f)->add(position), false, true, true, false, NULL, false);

    Material* forwardMaterial = Material::WATER;
    if (forwardHit != NULL)
        forwardMaterial = level->getBlock(forwardHit->getBlockPos())->mMaterial;

    Material* leftMaterial = Material::WATER;
    if (leftHit != NULL)
        leftMaterial = level->getBlock(leftHit->getBlockPos())->mMaterial;

    Material* rightMaterial = Material::WATER;
    if (rightHit != NULL)
        rightMaterial = level->getBlock(rightHit->getBlockPos())->mMaterial;

    if ((forwardHit != NULL && forwardHit->mType != HitResult::MISS)
        || forwardMaterial->blocksMotion()
        || (leftHit != NULL && leftHit->mType != HitResult::MISS)
        || leftMaterial->blocksMotion()
        || (rightHit != NULL && rightHit->mType != HitResult::MISS)
        || rightMaterial->blocksMotion())
    {
        Vec3* unchosen = Vec3::newTemp(0.0, 0.0, 0.0);
        Vec3* chosen = unchosen;
        double smallestAngle = 1.7976931348623157e+308;
        double furthestDistance = 2.2250738585072014e-308;
        bool clearDirectionFound = false;
        int offsetX;
        int offsetZ;

        for (offsetX = -1; offsetX <= 1; ++offsetX)
        {
            for (offsetZ = -1; offsetZ <= 1; ++offsetZ)
            {
                Vec3* candidate = Vec3::newTemp(position->x + (double)offsetX, position->y,
                                                position->z + (double)offsetZ)
                                      ->subtract(position)->normalize();
                HitResult* hit = level->clip(position,
                    candidate->multiply(mDistance)->add(position), false, true, true, false,
                    NULL, false);

                Material* material = Material::WATER;
                if (hit != NULL)
                    material = level->getBlock(hit->getBlockPos())->mMaterial;

                if ((hit != NULL && hit->mType != HitResult::MISS) || material->blocksMotion())
                {
                    double distance =
                        position->distanceToSqr((hit->getBlockPos() - position)->add(position));
                    if (!clearDirectionFound && distance > furthestDistance)
                    {
                        furthestDistance = distance;
                        chosen = candidate;
                    }
                }
                else
                {
                    double angle = std::acos(lookAngle->dot(candidate));
                    if (angle < smallestAngle)
                    {
                        clearDirectionFound = true;
                        smallestAngle = angle;
                        chosen = candidate;
                    }
                }
            }
        }

        if (chosen == unchosen)
            mSteerDirection->multiply(-1.0, -1.0, -1.0);
        else
            mSteerDirection = chosen->multiply(2.0);
    }

    Vec3* turnLeft = Vec3::newTemp(lookAngle->x, 0.0, lookAngle->z)->yRot(45.0f);
    Vec3* turnRight = Vec3::newTemp(lookAngle->x, 0.0, lookAngle->z)->yRot(-45.0f);

    HitResult* turnLeftHit = level->clip(position,
        turnLeft->multiply(mDistance)->add(position), false, true, true, false, NULL, false);
    HitResult* turnRightHit = level->clip(position,
        turnRight->multiply(mDistance)->add(position), false, true, true, false, NULL, false);

    if (turnLeftHit != NULL && turnLeftHit->mType != HitResult::MISS)
        mSteerDirection = mSteerDirection->add(turnRight->normalize()->multiply(1.2));

    if (turnRightHit != NULL && turnRightHit->mType != HitResult::MISS)
        mSteerDirection = mSteerDirection->add(turnLeft->normalize()->multiply(1.2));

    delete turnLeftHit;
    delete turnRightHit;

    HitResult* groundHit = level->clip(position,
        Vec3::newTemp(0.0, -1.0, 0.0)->multiply(2.0f)->add(position), false, true, true, false,
        NULL, false);
    bool groundBelow = groundHit != NULL && groundHit->mType != HitResult::MISS;

    BlockPos blockPos(mMob->getPos(0.0f));
    if (!mMob->followsLeader() && level->getBlock(blockPos.above()) == Blocks::air)
    {
        if (groundBelow)
        {
            double heightAboveBlock = mMob->mY - (double)blockPos.y;
            if (heightAboveBlock > 0.75)
                mSteerDirection = mSteerDirection->add(0.0, -0.1, 0.0);
            else if (heightAboveBlock < 0.25)
                mSteerDirection = mSteerDirection->add(0.0, 0.1, 0.0);
        }
        else
        {
            mSteerDirection = mSteerDirection->add(0.0, -0.25, 0.0);
        }
    }
    else if (groundBelow)
    {
        mSteerDirection = mSteerDirection->add(0.0, 0.5, 0.0);
    }
}

void RiverFollowingGoal::tick()
{
    determineSteerDirection();
    if (!mMob->followsLeader() || mMob->isLeader())
    {
        mSteerDirection = mSteerDirection->normalize()->multiply(mSpeed);
        float rotation = (float)std::atan2(mSteerDirection->x, mSteerDirection->z);
        mMob->mYRot = Mth::clampRotateDegrees(mMob->mYRot, -rotation * 57.295776f, 45.0f);
        if (mMob->isUnderLiquid(Material::WATER))
            mMob->mYd = 0.0;

        mMob->move(eMoverType_Self, mSteerDirection->x, mSteerDirection->y, mSteerDirection->z,
                   false);
        mMob->setMoving(true);
    }
}

bool RiverFollowingGoal::canContinueToUse()
{
    if (mMob->followsLeader())
        return false;

    BlockPos position(mMob->getPos(0.0f));
    return mMob->mLevel->getBiome(position)->getBiomeType() != Biome::eBiomeType_Ocean;
}

std::wstring RiverFollowingGoal::getName()
{
    return L"RiverFollowing";
}
