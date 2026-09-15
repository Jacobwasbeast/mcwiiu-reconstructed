#include "game/entity/ai/RangedAttackGoal.hpp"

#include "game/core/Mth.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/core/Sensing.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"

RangedAttackGoal::RangedAttackGoal(RangedAttackMob* rangedAttackMob, Mob* mob, double speedModifier, int attackInterval, float attackRadius)
    : mMob(NULL), mRangedAttackMob(NULL), mAttackTime(0), mUnknown(0),
      mSpeedModifier(0.0), mSeeTime(0), mAttackIntervalMin(0), mAttackIntervalMax(0),
      mAttackRadius(0.0f), mAttackRadiusSqr(0.0f), mTarget()
{
    _init(rangedAttackMob, mob, speedModifier, attackInterval, attackInterval, attackRadius);
}

RangedAttackGoal::RangedAttackGoal(RangedAttackMob* rangedAttackMob, Mob* mob, double speedModifier, int attackIntervalMin, int attackIntervalMax, float attackRadius)
    : mMob(NULL), mRangedAttackMob(NULL), mAttackTime(0), mUnknown(0),
      mSpeedModifier(0.0), mSeeTime(0), mAttackIntervalMin(0), mAttackIntervalMax(0),
      mAttackRadius(0.0f), mAttackRadiusSqr(0.0f), mTarget()
{
    _init(rangedAttackMob, mob, speedModifier, attackIntervalMin, attackIntervalMax, attackRadius);
}

void RangedAttackGoal::_init(RangedAttackMob* rangedAttackMob, Mob* mob, double speedModifier, int attackIntervalMin, int attackIntervalMax, float attackRadius)
{
    mRangedAttackMob = rangedAttackMob;
    mMob = mob;
    mSpeedModifier = speedModifier;
    mAttackIntervalMin = attackIntervalMin;
    mAttackIntervalMax = attackIntervalMax;
    mAttackRadius = attackRadius;
    mAttackRadiusSqr = attackRadius * attackRadius;
    setRequiredControlFlags(3);
    mTarget.reset();
    mSeeTime = 0;
    mAttackTime = -1;
}

RangedAttackGoal::~RangedAttackGoal()
{
}

bool RangedAttackGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
        return false;
    mTarget = target;
    return true;
}

bool RangedAttackGoal::canContinueToUse()
{
    return canUse() || !mMob->getNavigation()->isDone();
}

void RangedAttackGoal::stop()
{
    mSeeTime = 0;
    mTarget.reset();
    mAttackTime = -1;
}

void RangedAttackGoal::tick()
{
    if (!mTarget)
        return;

    double distance = mMob->distanceToSqr(mTarget->mX, mTarget->mBoundingBox->mMinY, mTarget->mZ);
    bool canSee = mMob->getSensing()->canSee(mTarget);
    if (canSee)
        ++mSeeTime;
    else
        mSeeTime = 0;

    if (!canSee || distance > (double)mAttackRadiusSqr || mSeeTime < 20)
        mMob->getNavigation()->moveTo(mTarget, mSpeedModifier);
    else
        mMob->getNavigation()->stop();

    static_cast<LookControl*>(mMob->getLookControl())->setLookAt(mTarget, 30.0f, 30.0f);

    --mAttackTime;
    if (mAttackTime < 0)
    {
        float distanceFactor = (float)(Mth::sqrt(distance) / (double)mAttackRadius);
        mAttackTime = Mth::floor(distanceFactor * (float)(mAttackIntervalMax - mAttackIntervalMin) +
                                 (float)mAttackIntervalMin);
    }
    else if (mAttackTime < 1 && canSee)
    {
        float distanceFactor = Mth::clamp((float)(Mth::sqrt(distance) / (double)mAttackRadius), 0.1f, 1.0f);
        mRangedAttackMob->performRangedAttack(mTarget, distanceFactor);
        mAttackTime = Mth::floor(distanceFactor * (float)(mAttackIntervalMax - mAttackIntervalMin) +
                                 (float)mAttackIntervalMin);
    }
}

std::wstring RangedAttackGoal::getName()
{
    return L"RangedAttack";
}
