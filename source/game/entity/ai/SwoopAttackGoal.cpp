#include "game/entity/ai/SwoopAttackGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"

SwoopAttackGoal::SwoopAttackGoal(Mob* mob, float speedModifier,
                                 Goal::FloatRange attackDistance)
    : Goal()
    , mMob(mob)
    , mSpeedModifier(speedModifier)
    , mAttackDistanceMin(attackDistance.min)
    , mAttackDistanceMax(attackDistance.max)
    , mAttackInterval(0)
    , mSwoopTicks(0)
    , mSwooping(false)
{
    setRequiredControlFlags(3);
}

SwoopAttackGoal::~SwoopAttackGoal()
{
}

bool SwoopAttackGoal::canUse()
{
    if (mMob->getMoveControl() == NULL)
    {
        return false;
    }

    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
    {
        return false;
    }

    if (mAttackInterval < 1)
    {
        int distance = (int)(mMob->getRandom()->nextFloat()
            * (mAttackDistanceMax - mAttackDistanceMin) + mAttackDistanceMin);
        mAttackInterval = distance * 20;
        return true;
    }

    --mAttackInterval;
    return false;
}

void SwoopAttackGoal::start()
{
    mSwooping = false;
    mMob->playSound(SoundEvents::PHANTOM_SWOOP, 10.0f, 1.0f);
    mSwoopTicks = 0;
}

void SwoopAttackGoal::stop()
{
    PathNavigation* navigation = mMob->getNavigation();
    if (navigation != NULL)
    {
        navigation->stop();
    }
    else
    {
        mMob->getMoveControl()->wait();
    }
}

bool SwoopAttackGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    return target && !mSwooping && !mMob->mHorizontalCollision && mMob->mHurtTime < 1;
}

void SwoopAttackGoal::tick()
{
    --mSwoopTicks;
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (target && mSwoopTicks < 1)
    {
        Vec3* targetPosition = target->getPos(0.0f);
        Vec3* wantedPosition = Vec3::newTemp(targetPosition->x,
                                             targetPosition->y + (double)target->mBbHeight,
                                             targetPosition->z);
        PathNavigation* navigation = mMob->getNavigation();
        if (navigation == NULL)
        {
            mMob->getMoveControl()->setWantedPosition(wantedPosition->x, wantedPosition->y,
                                                      wantedPosition->z,
                                                      (double)mSpeedModifier);
        }
        else
        {
            navigation->moveTo(wantedPosition->x, wantedPosition->y, wantedPosition->z,
                               (double)mSpeedModifier);
        }

        if (mMob->mBoundingBox->grow(0.2)->intersects(
                target->mBoundingBox->mMinX, target->mBoundingBox->mMinY,
                target->mBoundingBox->mMinZ, target->mBoundingBox->mMaxX,
                target->mBoundingBox->mMaxY, target->mBoundingBox->mMaxZ))
        {
            mMob->doHurtTarget(target);
            mSwooping = true;
            mMob->playSound(SoundEvents::PHANTOM_BITE, 1.0f, 1.0f);
        }

        double distance = mMob->distanceToSqr(target->mX, target->mBoundingBox->mMinY,
                                              target->mZ);
        if (distance <= 1024.0)
        {
            if (distance > 256.0)
            {
                mSwoopTicks = 2;
            }
        }
        else
        {
            mSwoopTicks = 5;
        }
    }
}

std::wstring SwoopAttackGoal::getName()
{
    return L"Swooping Attack!";
}
