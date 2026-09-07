#include "game/entity/ai/MoveTowardsTargetGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/RandomPos.hpp"

MoveTowardsTargetGoal::MoveTowardsTargetGoal(PathfinderMob* mob, double speedModifier,
                                             float within)
    : mMob(mob),
      mTarget(),
      mWantedPosition(0.25, 0.25, 0.25),
      mSpeedModifier(speedModifier),
      mWithin(within)
{
    setRequiredControlFlags(1);
}

MoveTowardsTargetGoal::~MoveTowardsTargetGoal()
{
}

bool MoveTowardsTargetGoal::canUse()
{
    mTarget = mMob->getTarget();
    if (!mTarget)
        return false;

    if (mMob->distanceToSqr(mTarget) > static_cast<double>(mWithin * mWithin))
        return false;

    boost::shared_ptr<PathfinderMob> mob =
        boost::dynamic_pointer_cast<PathfinderMob>(mMob->shared_from_this());
    Vec3* targetPosition = Vec3::newTemp(mTarget->mX, mTarget->mY, mTarget->mZ);
    Vec3* position = RandomPos::getPosTowards(mob, 16, 7, targetPosition);
    if (position == NULL)
        return false;

    mWantedPosition = *position;
    return true;
}

bool MoveTowardsTargetGoal::canContinueToUse()
{
    if (!mTarget)
        return false;

    if (!mTarget->isAlive() || mMob->getNavigation()->isDone())
        return false;

    return mMob->distanceToSqr(mTarget) < static_cast<double>(mWithin * mWithin);
}

void MoveTowardsTargetGoal::start()
{
    mMob->getNavigation()->moveTo(mWantedPosition.x, mWantedPosition.y,
                                  mWantedPosition.z, mSpeedModifier);
}

void MoveTowardsTargetGoal::stop()
{
    mTarget.reset();
}

std::wstring MoveTowardsTargetGoal::getName()
{
    return std::wstring(L"MoveTowardsTarget");
}
