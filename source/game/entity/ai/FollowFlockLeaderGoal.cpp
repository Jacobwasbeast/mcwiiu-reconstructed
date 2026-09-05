#include "game/entity/ai/FollowFlockLeaderGoal.hpp"

#include <cmath>
#include <typeinfo>
#include <vector>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

FollowFlockLeaderGoal::FollowFlockLeaderGoal(Mob* mob, bool inWater)
    : Goal(), mInterval(11), mMob(mob), mLeader(), mTimeToRecalcPath(11), mInWater(inWater)
{
}

FollowFlockLeaderGoal::~FollowFlockLeaderGoal()
{
}

bool FollowFlockLeaderGoal::canUse()
{
    if (mMob->isLeader() || mMob->followsLeader()
        || mMob->isInWaterOrBubble() != mInWater)
    {
        return false;
    }

    std::vector<boost::shared_ptr<Entity> >* entities =
        mMob->mLevel->getEntitiesOfClass(typeid(*mMob), mMob->mBoundingBox->grow(8.0));
    if (entities->size() < 2)
    {
        delete entities;
        return false;
    }

    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Mob> candidate = boost::static_pointer_cast<Mob>(*it);
        if (candidate->shouldFollow() && candidate.get() != mMob)
        {
            mLeader = candidate;
            delete entities;
            return true;
        }
    }

    for (it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Mob> candidate = boost::static_pointer_cast<Mob>(*it);
        if (candidate.get() != mMob && !candidate->followsLeader() && !candidate->isLeader())
        {
            candidate->setLeader(true);
            mLeader = candidate;
            delete entities;
            return true;
        }
    }

    delete entities;
    return false;
}

bool FollowFlockLeaderGoal::canContinueToUse()
{
    if (!mLeader || mMob->isInWaterOrBubble() != mInWater)
    {
        return false;
    }
    if (!mLeader->isAlive() || !mLeader->isLeader())
    {
        return false;
    }

    return mMob->distanceToSqr(boost::static_pointer_cast<Entity>(mLeader)) <= 121.0;
}

void FollowFlockLeaderGoal::start()
{
    mMob->setFollowsLeader(true);
    mTimeToRecalcPath = 0;
}

void FollowFlockLeaderGoal::stop()
{
    mMob->setFollowsLeader(false);
    mLeader.reset();
}

BlockPos FollowFlockLeaderGoal::GetTargetPosition(boost::shared_ptr<Mob> leader)
{
    Vec3* view = leader->getViewVector(1.0f);
    Vec3* side = Vec3::newTemp(0.0, 1.0, 0.0)->cross(view)->normalize();
    Vec3* delta = mMob->getPos(0.0f)->subtract(leader->getPos(0.0f))->normalize();
    const double sideAmount = side->dot(delta);
    Vec3* position = Vec3::newTemp(
        std::floor(leader->getPos(0.0f)->x) + 0.5,
        std::floor(leader->getPos(0.0f)->y) + 0.5,
        std::floor(leader->getPos(0.0f)->z) + 0.5);
    position->add(side->multiply(sideAmount * 3.0));
    position->add(leader->getViewVector(1.0f)->multiply(0.0625));

    BlockPos target(position);
    if (leader->getNavigation()->isRiverFollowing())
    {
        return target;
    }
    return target.offset(0, -1, 0);
}

void FollowFlockLeaderGoal::tick()
{
    if (!mLeader || !mLeader->isAlive())
    {
        mTimeToRecalcPath = mInterval;
        return;
    }

    --mTimeToRecalcPath;
    if (mTimeToRecalcPath < 1)
    {
        mTimeToRecalcPath = mInterval;
        BlockPos target = GetTargetPosition(mLeader);
        mMob->getNavigation()->moveTo((double)target.x, (double)target.y,
                                      (double)target.z, 1.0);
    }
}

std::wstring FollowFlockLeaderGoal::getName()
{
    return std::wstring(L"FollowFlock");
}
