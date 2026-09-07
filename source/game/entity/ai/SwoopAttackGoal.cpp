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
    if (mMob->getNavigation() == NULL)
    {
        return false;
    }

    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
    {
        return false;
    }

    if (mMob->getNavigation()->isDone())
    {
        float distance = mMob->mRandom->nextFloat() *
                         (mAttackDistanceMax - mAttackDistanceMin) +
                         mAttackDistanceMin;
        mAttackInterval = (int)(distance * 20.0f);
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
    return target && !mSwooping && !mMob->mHorizontalCollision &&
           mMob->getMoveControl()->hasWanted();
}

void SwoopAttackGoal::tick()
{
    --mSwoopTicks;
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (target && mSwoopTicks < 1)
    {
        Vec3* targetPosition = target->getPos(2.0f)->add(0.0,
                                                         (double)target->mBbHeight,
                                                         0.0);
        PathNavigation* navigation = mMob->getNavigation();
        if (navigation == NULL)
        {
            mMob->getMoveControl()->setWantedPosition(targetPosition->x,
                                                       targetPosition->y,
                                                       targetPosition->z,
                                                       (double)mSpeedModifier);
        }
        else
        {
            navigation->moveTo(targetPosition->x, targetPosition->y,
                               targetPosition->z, (double)mSpeedModifier);
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

        double distance = mMob->distanceToSqr(target->mX,
                                               target->mBoundingBox->mMinY,
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
    return std::wstring(L"Swooping_Attack_");
}
