#include "game/entity/ai/SwimWithMobGoal.hpp"

#include <cmath>
#include <vector>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

SwimWithMobGoal::SwimWithMobGoal(Mob* mob, float speedModifier, float maxDistance,
                                 int searchDistance,
                                 arrayWithLength<unsigned long long> entityTypes)
    : mMob(mob), mSpeedModifier(speedModifier), mWantedMob(), mCooldown(0), mState(0),
      mEntityTypes(entityTypes), mMaxDistance(maxDistance * maxDistance),
      mSearchDistance(searchDistance)
{
    setRequiredControlFlags(3);
}

SwimWithMobGoal::~SwimWithMobGoal()
{
    delete[] mEntityTypes.data;
}

bool SwimWithMobGoal::canSwimWith(boost::shared_ptr<Entity> entity)
{
    if (mEntityTypes.length == 0)
        return true;

    for (unsigned int i = 0; i < mEntityTypes.length; ++i)
    {
        if ((entity->GetType() & mEntityTypes.data[i]) == mEntityTypes.data[i])
            return true;
    }

    return false;
}

bool SwimWithMobGoal::setWantedMob()
{
    AABB* box = mMob->mBoundingBox->grow((double)mSearchDistance);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mMob->mLevel->getEntities(mMob->shared_from_this(), box, NULL);
    std::vector<boost::shared_ptr<Entity> > candidates;

    if (entities != NULL)
    {
        std::vector<boost::shared_ptr<Entity> >::iterator it;
        for (it = entities->begin(); it != entities->end(); ++it)
        {
            boost::shared_ptr<Entity> entity = *it;
            if (entity && entity->isAlive() && canSwimWith(entity))
                candidates.push_back(entity);
        }
        delete entities;
    }

    if (candidates.empty())
        return false;

    mWantedMob = candidates.at((unsigned int)mMob->mRandom->nextInt((int)candidates.size()));
    return true;
}

bool SwimWithMobGoal::canUse()
{
    if (!mMob->isInWater() || mMob->mRandom->nextInt(10) != 0)
        return false;

    return setWantedMob();
}

bool SwimWithMobGoal::canContinueToUse()
{
    if (!mWantedMob || !mWantedMob->isAlive())
        return false;

    if (mMob->distanceToSqr(mWantedMob) > (double)mMaxDistance)
        return false;

    if (!mWantedMob->isInWater() && mMob->mRandom->nextInt(30) == 0)
        return false;

    return true;
}

bool SwimWithMobGoal::canInterrupt()
{
    return true;
}

void SwimWithMobGoal::start()
{
    mCooldown = 0;
    mState = 0;
}

void SwimWithMobGoal::stop()
{
    mWantedMob.reset();
    mCooldown = 0;
}

BlockPos SwimWithMobGoal::GetTargetPosition(boost::shared_ptr<Entity> entity, Vec3* direction,
                                            double distance, double offset)
{
    Vec3* position = Vec3::newTemp(std::floor(entity->getPos(1.0f)->x),
                                   std::floor(entity->getPos(1.0f)->y),
                                   std::floor(entity->getPos(1.0f)->z));
    position->add(0.5, 0.5, 0.5);
    position->add(direction->multiply(distance * 3.0));
    position->add(entity->getViewVector(1.0f)->multiply(offset));
    return BlockPos(position).offset(0, -1, 0);
}

void SwimWithMobGoal::tick()
{
    if (!mWantedMob || !mWantedMob->isAlive())
        return;

    float speed = mSpeedModifier;
    if (mState != 1)
        speed *= 0.5f;

    --mCooldown;
    if (mCooldown > 0 && !mMob->getNavigation()->isDone())
        return;

    mCooldown = 10;
    Vec3* direction = Vec3::newTemp(0.0, 1.0, 0.0)->cross(mWantedMob->getViewVector(1.0f))->normalize();
    Vec3* delta = mMob->getPos(1.0f)->subtract(mWantedMob->getPos(1.0f))->normalize();
    double side = direction->dot(delta);
    if (side > -2.220446049250313e-16 && side < 0.2)
        side = 0.2;

    double distance = mMob->distanceToSqr(mWantedMob);
    if (mState == 0)
    {
        if (distance >= 6.25)
        {
            BlockPos target = GetTargetPosition(mWantedMob, direction, side, 2.0);
            mMob->getNavigation()->moveTo((double)target.x, (double)target.y,
                                          (double)target.z, (double)(speed + 1.0f));
        }
        else
        {
            mMob->getNavigation()->stop();
            mCooldown = 0;
            mState = 1;
        }
    }
    else if (mState == 1)
    {
        if (distance > 144.0)
        {
            mMob->getNavigation()->stop();
            mCooldown = 0;
            mState = 0;
        }
        else
        {
            BlockPos target = GetTargetPosition(mWantedMob, direction, side, 6.0);
            mMob->getNavigation()->moveTo((double)target.x, (double)target.y,
                                          (double)target.z, (double)(speed + 1.0f));
        }
    }
}

std::wstring SwimWithMobGoal::getName()
{
    return std::wstring(L"SwimWithMob");
}
