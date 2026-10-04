#include "game/entity/ai/FollowFlockLeaderGoal.hpp"

#include <typeinfo>
#include <vector>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

static int sIntervalOffset = 0;

FollowFlockLeaderGoal::FollowFlockLeaderGoal(Mob* mob, bool inWater)
    : Goal(),
      mInterval(sIntervalOffset + 11),
      mMob(mob),
      mLeader(),
      mTimeToRecalcPath(sIntervalOffset++ + 11),
      mInWater(inWater)
{
    if (sIntervalOffset > 10)
    {
        sIntervalOffset = 0;
    }
}

bool FollowFlockLeaderGoal::canUse()
{
    if (mMob->isLeader() || mMob->followsLeader() || mMob->isInWaterOrBubble() != mInWater)
    {
        return false;
    }

    std::vector<boost::shared_ptr<Entity> >* entities =
        mMob->mLevel->getEntitiesOfClass(typeid(*mMob), mMob->mBoundingBox->grow(8.0));
    if (entities->size() > 1)
    {
        boost::shared_ptr<Mob> self = boost::static_pointer_cast<Mob>(mMob->shared_from_this());

        std::vector<boost::shared_ptr<Entity> >::iterator it;
        for (it = entities->begin(); it != entities->end(); ++it)
        {
            boost::shared_ptr<Mob> candidate = boost::static_pointer_cast<Mob>(*it);
            if (candidate->shouldFollow() && candidate != self)
            {
                ++candidate->mFlockSize;
                mLeader = candidate;
                delete entities;
                return true;
            }
        }

        for (it = entities->begin(); it != entities->end(); ++it)
        {
            boost::shared_ptr<Mob> candidate = boost::static_pointer_cast<Mob>(*it);
            if (candidate != self && !candidate->followsLeader() && !candidate->isLeader())
            {
                candidate->setLeader(true);
                ++candidate->mFlockSize;
                mLeader = candidate;
                delete entities;
                return true;
            }
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

    return mMob->distanceToSqr(mLeader) <= 121.0;
}

void FollowFlockLeaderGoal::start()
{
    mMob->setFollowsLeader(true);
    mTimeToRecalcPath = 0;
}

void FollowFlockLeaderGoal::stop()
{
    mMob->setFollowsLeader(false);
    if (mLeader)
    {
        --mLeader->mFlockSize;
        mLeader.reset();
    }
}

BlockPos FollowFlockLeaderGoal::GetTargetPosition(boost::shared_ptr<Mob> leader)
{
    Vec3* side = leader->getLookAngle()->cross(Vec3::newTemp(0.0, 1.0, 0.0))->normalize();
    Vec3* mobPos = mMob->getPos(0.0f);
    Vec3* delta = mobPos->subtract(leader->getPos(0.0f))->normalize();
    double sideAmount = side->dot(delta);

    double leaderX = leader->getPos(0.0f)->x;
    int x = static_cast<int>(leaderX);
    if (leaderX < static_cast<double>(x)) --x;

    double leaderY = leader->getPos(0.0f)->y;
    int y = static_cast<int>(leaderY);
    if (leaderY < static_cast<double>(y)) --y;

    double leaderZ = leader->getPos(0.0f)->z;
    int z = static_cast<int>(leaderZ);
    if (leaderZ < static_cast<double>(z)) --z;

    Vec3* center = Vec3::newTemp(static_cast<double>(x), static_cast<double>(y), static_cast<double>(z))
                       ->add(0.5, 0.5, 0.5);
    Vec3* position = center->add(leader->getLookAngle()->multiply(6.0))->add(side->multiply(sideAmount * 3.0));

    BlockPos target(position);
    if (leader->getNavigation()->isRiverFollowing())
    {
        return target;
    }

    return target.offset(0, -1, 0);
}

void FollowFlockLeaderGoal::tick()
{
    if (!mLeader || !mLeader->isMoving())
    {
        mTimeToRecalcPath = mInterval;
        return;
    }

    if (mTimeToRecalcPath - 1 > 0)
    {
        --mTimeToRecalcPath;
        return;
    }
    mTimeToRecalcPath = mInterval;
    boost::shared_ptr<Mob> self = boost::static_pointer_cast<Mob>(mMob->shared_from_this());
    BlockPos target = GetTargetPosition(mLeader);
    self->getNavigation()->moveTo((double)target.x, (double)target.y, (double)target.z, 1.0);
}

std::wstring FollowFlockLeaderGoal::getName()
{
    return std::wstring(L"FollowFlock");
}
