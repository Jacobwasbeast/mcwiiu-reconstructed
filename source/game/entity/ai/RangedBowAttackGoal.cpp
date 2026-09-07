#include "game/entity/ai/RangedBowAttackGoal.hpp"

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/core/Sensing.hpp"
#include "game/util/Random.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/Monster.hpp"
#include "game/entity/mob/RangedAttackMob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

extern int BowItem_GetMaxBowDuration() __asm("GetMaxBowDuration__7BowItemSFv");
extern float BowItem_getPowerForTime(int charge) __asm("getPowerForTime__7BowItemSFi");

RangedBowAttackGoal::RangedBowAttackGoal(Monster* mob, double speedModifier, int attackInterval,
                                         float attackRadius)
    : Goal(), mMob(mob), mSpeedModifier(speedModifier), mAttackInterval(attackInterval),
      mAttackRadiusSqr(attackRadius * attackRadius), mAttackTime(-1), mSeeTime(0),
      mStrafingClockwise(false), mStrafingBackwards(false), mStrafingTime(-1)
{
    setRequiredControlFlags(3);
}

RangedBowAttackGoal::~RangedBowAttackGoal()
{
}

void RangedBowAttackGoal::setMinAttackInterval(int attackInterval)
{
    mAttackInterval = attackInterval;
}

bool RangedBowAttackGoal::isHoldingBow()
{
    not_null_ptr<ItemInstance> item = mMob->getMainHandItem();
    return !item->isEmpty() && item->getItem() == Items::bow;
}

bool RangedBowAttackGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    return target != NULL && isHoldingBow();
}

bool RangedBowAttackGoal::canContinueToUse()
{
    if (!Goal::canInterrupt())
        return !mMob->getNavigation()->isDone();
    return isHoldingBow();
}

void RangedBowAttackGoal::start()
{
    Goal::start();
    RangedAttackMob* rangedAttackMob = dynamic_cast<RangedAttackMob*>(mMob);
    rangedAttackMob->setAggressive(true);
}

void RangedBowAttackGoal::stop()
{
    Goal::stop();
    RangedAttackMob* rangedAttackMob = dynamic_cast<RangedAttackMob*>(mMob);
    rangedAttackMob->setAggressive(false);
    mMob->stopUsingItem();
    mSeeTime = 0;
    mAttackTime = -1;
}

void RangedBowAttackGoal::tick()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
        return;

    double distance = target->distanceToSqr(mMob->mX, mMob->mY, mMob->mZ);
    bool canSee = mMob->getSensing()->canSee(target);
    if (canSee)
        ++mSeeTime;
    else
        mSeeTime = 0;

    if (!canSee || distance > static_cast<double>(mAttackRadiusSqr))
    {
        --mSeeTime;
        mMob->getNavigation()->moveTo(target, mSpeedModifier);
    }
    else if (mSeeTime >= 20)
    {
        mMob->getNavigation()->stop();
    }

    if (canSee)
        mMob->getLookControl()->setLookAt(target, 30.0f, 30.0f);
    else
        mMob->lookAt(target, 30.0f, 30.0f);

    if (mSeeTime < 20)
    {
        mStrafingTime = -1;
    }
    else
    {
        ++mStrafingTime;
        if (mStrafingTime < 20)
        {
            if (distance < static_cast<double>(mAttackRadiusSqr * 0.25f))
                mStrafingBackwards = true;
            float forwards = mStrafingBackwards ? -0.5f : 0.5f;
            float sideways = mStrafingClockwise ? 0.5f : -0.5f;
            mMob->getMoveControl()->strafe(forwards, sideways);
        }
        else
        {
            mStrafingTime = 0;
            if (mMob->mRandom->nextFloat() < 0.3f)
                mStrafingClockwise = !mStrafingClockwise;
            if (mMob->mRandom->nextFloat() < 0.3f)
                mStrafingBackwards = !mStrafingBackwards;
        }
    }

    --mAttackTime;
    if (!mMob->isUsingItem())
    {
        if (canSee && distance <= static_cast<double>(mAttackRadiusSqr))
            mMob->startUsingItem(InteractionHand::eInteractionHand_MainHand);
    }
    else if (mMob->getTicksUsingItem() >= BowItem_GetMaxBowDuration())
    {
        mMob->stopUsingItem();
        RangedAttackMob* rangedAttackMob = dynamic_cast<RangedAttackMob*>(mMob);
        rangedAttackMob->performRangedAttack(target, BowItem_getPowerForTime(mMob->getTicksUsingItem()));
        mAttackTime = mAttackInterval;
    }
}

std::wstring RangedBowAttackGoal::getName()
{
    return std::wstring(L"RangedBowAtack");
}
