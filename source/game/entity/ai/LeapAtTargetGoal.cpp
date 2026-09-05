#include "game/entity/ai/LeapAtTargetGoal.hpp"

#include <cmath>

#include "game/core/Difficulty.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

LeapAtTargetGoal::LeapAtTargetGoal(Mob* mob, float yd)
    : mMob(mob), mTarget(), mYd(yd)
{
    setRequiredControlFlags(5);
}

LeapAtTargetGoal::~LeapAtTargetGoal()
{
}

bool LeapAtTargetGoal::canUse()
{
    mTarget = mMob->getTarget();
    if (!mTarget)
    {
        return false;
    }
    double distance = mMob->distanceToSqr(mTarget);
    return distance >= 4.0 && distance <= 16.0 && mMob->mOnGround
        && mMob->getRandom()->nextInt(5) == 0;
}

bool LeapAtTargetGoal::canContinueToUse()
{
    if (!mTarget)
    {
        return false;
    }
    if ((mTarget->GetType() & ENTITY_TYPE_LIVING_ENTITY) != ENTITY_TYPE_LIVING_ENTITY)
    {
        return !mMob->mOnGround;
    }
    return mMob->mLevel->getDifficulty() != Difficulty::PEACEFUL && !mMob->mOnGround;
}

void LeapAtTargetGoal::start()
{
    double dx = mTarget->mX - mMob->mX;
    double dz = mTarget->mZ - mMob->mZ;
    double distance = std::sqrt(dx * dx + dz * dz);
    if (distance >= 0.0001)
    {
        mMob->mXd += dx / distance * 0.5 + mMob->mXd * 0.2;
        mMob->mYd = static_cast<double>(mYd);
        mMob->mZd += dz / distance * 0.5 + mMob->mZd * 0.2;
    }
    else
    {
        mMob->mYd = static_cast<double>(mYd);
    }
}

std::wstring LeapAtTargetGoal::getName()
{
    return std::wstring(L"LeapAtTarget");
}
