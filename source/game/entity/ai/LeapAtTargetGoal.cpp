#include "game/entity/ai/LeapAtTargetGoal.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

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
    mTarget.reset();
    mMob = 0;
}

bool LeapAtTargetGoal::canUse()
{
    mTarget = mMob->getTarget();
    if (!mTarget.lock())
    {
        return false;
    }
    double distance = mMob->distanceToSqr(mTarget.lock());
    return distance >= 4.0 && distance <= 16.0 && mMob->mOnGround
        && mMob->getRandom()->nextInt(5) == 0;
}

bool LeapAtTargetGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mTarget.lock();
    if (!target)
    {
        return false;
    }
    if ((target->GetType() & EntityType::PLAYER) != EntityType::PLAYER)
    {
        return !mMob->mOnGround;
    }
    return target->mLevel->getDifficulty() != Difficulty::PEACEFUL && !mMob->mOnGround;
}

void LeapAtTargetGoal::start()
{
    double dx = mTarget.lock()->mX - mMob->mX;
    double dz = mTarget.lock()->mZ - mMob->mZ;
    double distance = std::sqrt(dx * dx + dz * dz);
    if (distance >= 0.0001)
    {
        mMob->mXd += dx / distance * 0.5 * 0.800000011920929 + mMob->mXd * 0.2;
        mMob->mZd += dz / distance * 0.5 * 0.800000011920929 + mMob->mZd * 0.2;
    }
    mMob->mYd = static_cast<double>(mYd);
}

std::wstring LeapAtTargetGoal::getName()
{
    return std::wstring(L"LeapAtTarget");
}
