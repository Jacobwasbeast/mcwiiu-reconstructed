#include "game/entity/ai/FollowMobGoal.hpp"

#include <typeinfo>
#include <vector>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPathTypes.hpp"

FollowMobGoal::FollowPredicate::FollowPredicate(Mob* mob)
    : mMob(mob)
{
}

FollowMobGoal::FollowPredicate::~FollowPredicate()
{
}

bool FollowMobGoal::FollowPredicate::apply(boost::shared_ptr<Entity> entity) const
{
    return entity && mMob->GetType() != entity->GetType();
}

FollowMobGoal::FollowMobGoal(Mob* mob, double speedModifier, float stopDistance, float areaSize)
    : mMob(mob), mPredicate(new FollowPredicate(mob)), mTarget(), mSpeedModifier(speedModifier),
      mNavigation(mob->getNavigation()), mTimeToRecalcPath(0), mStopDistance(stopDistance),
      mOldWaterCost(0.0f), mAreaSize(areaSize)
{
    setRequiredControlFlags(3);
}

FollowMobGoal::~FollowMobGoal()
{
    delete mPredicate;
}

bool FollowMobGoal::canUse()
{
    AABB* searchBox = mMob->mBoundingBox->grow((double)mAreaSize);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mMob->mLevel->getEntitiesOfClass(typeid(Mob), searchBox, mPredicate, false);

    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Mob> candidate = boost::static_pointer_cast<Mob>(*it);
        if (candidate && !candidate->isBaby())
        {
            mTarget = *it;
            delete entities;
            return true;
        }
    }

    delete entities;
    return false;
}

bool FollowMobGoal::canContinueToUse()
{
    if (!mTarget || mNavigation->isDone())
        return false;
    return mMob->distanceToSqr(mTarget) > (double)(mStopDistance * mStopDistance);
}

void FollowMobGoal::start()
{
    mTimeToRecalcPath = 0;
    mOldWaterCost = mMob->getPathfindingMalus(BlockPathTypes::WATER);
    mMob->setPathfindingMalus(BlockPathTypes::WATER, 0.0f);
}

void FollowMobGoal::stop()
{
    mTarget.reset();
    mNavigation->stop();
    mMob->setPathfindingMalus(BlockPathTypes::WATER, mOldWaterCost);
}

void FollowMobGoal::tick()
{
    if (!mTarget)
        return;

    mMob->getLookControl()->setLookAt(mTarget, 10.0f, 30.0f);
    --mTimeToRecalcPath;
    if (mTimeToRecalcPath < 1)
    {
        mTimeToRecalcPath = 10;
        if (mMob->distanceToSqr(mTarget) > (double)(mStopDistance * mStopDistance))
            mNavigation->moveTo(mTarget, mSpeedModifier);
        else
            mNavigation->stop();
    }
}

std::wstring FollowMobGoal::getName()
{
    return std::wstring(L"FollowMobGoal");
}
