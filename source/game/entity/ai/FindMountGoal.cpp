#include "game/entity/ai/FindMountGoal.hpp"

#include <typeinfo>
#include <vector>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

FindMountGoal::FindMountGoal(Mob& mob, float targetDistance, int startDelay,
                             bool avoidWater, bool targetNeeded, float mountDistance)
    : mTarget(), mTargetDistance(targetDistance), mTimeToRecalcPath(0),
      mStartCounter(0), mStartDelay(startDelay), mAvoidWater(avoidWater),
      mTargetNeeded(targetNeeded), mMountDistance(mountDistance), mMob(&mob)
{
    setRequiredControlFlags(3);
}

FindMountGoal::~FindMountGoal()
{
    mTarget.reset();
}

bool FindMountGoal::canUse()
{
    if (!mMob->isAlive() || mMob->isPassenger())
        return false;

    if (!static_cast<Zombie*>(mMob)->isJockey())
        return false;

    AABB* searchBox = mMob->mBoundingBox->grow((double)mTargetDistance);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mMob->mLevel->getEntitiesOfClass(typeid(Entity), searchBox);

    boost::shared_ptr<Entity> nearest;
    double nearestDistance = 0.0;
    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> candidate = *it;
        if (!candidate || candidate.get() == mMob || !candidate->isAlive())
            continue;

        unsigned long long type = candidate->GetType();
        bool mountType = (type == 0x000000000D680004ULL) ||
                         (type == 0x000000005D560041ULL) ||
                         ((type & 0x000000000D690000ULL) == 0x000000000D690000ULL) ||
                         ((type & 0x000000005D690000ULL) == 0x000000005D690000ULL);
        if (!mountType || !candidate->canAddPassenger(mMob->shared_from_this()))
            continue;

        double distance = mMob->distanceToSqr(candidate);
        if (!nearest || distance < nearestDistance)
        {
            nearest = candidate;
            nearestDistance = distance;
        }
    }

    delete entities;
    if (!nearest)
        return false;

    mTarget = nearest;
    return true;
}

bool FindMountGoal::canContinueToUse()
{
    if (!mTarget || !mTarget->isAlive() || !mMob->isAlive() || mMob->isPassenger())
        return false;
    return true;
}

void FindMountGoal::start()
{
    mTimeToRecalcPath = 0;
}

void FindMountGoal::stop()
{
    mMob->getNavigation()->stop();
    mTarget.reset();
    mStartCounter = 0;
}

double FindMountGoal::getAttackReachSqr()
{
    float reach = mMob->mBbWidth + mMob->mBbWidth + 2.0f;
    reach += mTarget->mBbWidth;
    return (double)(reach * reach);
}

void FindMountGoal::tick()
{
    if (!mTarget)
        return;

    mMob->getLookControl()->setLookAt(mTarget, 30.0f, 30.0f);
    if (mMob->distanceToSqr(mTarget) < getAttackReachSqr())
    {
        mMob->startRiding(mTarget, true);
        return;
    }

    --mTimeToRecalcPath;
    if (mTimeToRecalcPath < 1)
    {
        mTimeToRecalcPath = mMob->mRandom->nextInt(7) + 4;
        mMob->getNavigation()->moveTo(mTarget, 1.0);
    }
}

std::wstring FindMountGoal::getName()
{
    return std::wstring(L"FindMountGoal");
}
