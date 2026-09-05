#include "game/entity/ai/FollowBoatGoal.hpp"

#include <typeinfo>
#include <vector>

#include "game/core/Boat.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"

FollowBoatGoal::FollowBoatGoal(Dolphin* dolphin)
    : mSearchDistance(4), mMaxDistance(12), mCooldown(0), mState(0), mTarget(),
      mDolphin(dolphin)
{
}

FollowBoatGoal::~FollowBoatGoal()
{
}

bool FollowBoatGoal::canUse()
{
    Entity* dolphin = (Entity*)mDolphin;
    if (dolphin == NULL || dolphin->mLevel == NULL || dolphin->mBoundingBox == NULL)
        return false;
    AABB* searchBox = dolphin->mBoundingBox->grow(5.0);
    std::vector<boost::shared_ptr<Entity> >* boats =
        dolphin->mLevel->getEntitiesOfClass(typeid(Boat), searchBox);
    bool found = false;
    if (boats != NULL)
    {
        std::vector<boost::shared_ptr<Entity> >::iterator it;
        for (it = boats->begin(); it != boats->end(); ++it)
        {
            boost::shared_ptr<Entity> passenger = (*it)->getControllingPassenger();
            if (passenger && (passenger->GetType() & ENTITY_TYPE_LIVING_ENTITY) ==
                ENTITY_TYPE_LIVING_ENTITY &&
                (Mth::abs((float)passenger->mXd) > 5.0f ||
                 Mth::abs((float)passenger->mZd) > 5.0f))
            {
                found = true;
                break;
            }
        }
        delete boats;
    }
    return !mTarget && found;
}

bool FollowBoatGoal::canContinueToUse()
{
    if (!mTarget || !mTarget->isAlive())
        return false;
    return Mth::abs((float)mTarget->mXd) > 5.0f ||
           Mth::abs((float)mTarget->mZd) > 5.0f;
}

bool FollowBoatGoal::canInterrupt()
{
    return true;
}

void FollowBoatGoal::start()
{
    Entity* dolphin = (Entity*)mDolphin;
    if (dolphin == NULL || dolphin->mLevel == NULL || dolphin->mBoundingBox == NULL)
        return;
    AABB* searchBox = dolphin->mBoundingBox->grow(5.0);
    std::vector<boost::shared_ptr<Entity> >* boats =
        dolphin->mLevel->getEntitiesOfClass(typeid(Boat), searchBox);
    if (boats != NULL)
    {
        std::vector<boost::shared_ptr<Entity> >::iterator it;
        for (it = boats->begin(); it != boats->end(); ++it)
        {
            boost::shared_ptr<Entity> passenger = (*it)->getControllingPassenger();
            if (passenger && (passenger->GetType() & ENTITY_TYPE_LIVING_ENTITY) ==
                ENTITY_TYPE_LIVING_ENTITY)
            {
                mTarget = passenger;
                break;
            }
        }
        delete boats;
    }
    mCooldown = 0;
    mState = 0;
}

void FollowBoatGoal::stop()
{
    mTarget.reset();
}

void FollowBoatGoal::tick()
{
    Entity* dolphin = (Entity*)mDolphin;
    if (dolphin == NULL || !mTarget)
        return;
    bool moving = Mth::abs((float)mTarget->mXd) > 0.0f ||
                  Mth::abs((float)mTarget->mZd) > 0.0f;
    float speed = mState == 1 && moving ? 0.2f : 0.15f;
    dolphin->moveRelative((float)mTarget->mXd, (float)mTarget->mYd,
                          (float)mTarget->mZd, speed);
    dolphin->lerpMotion(dolphin->mXd * 0.9, dolphin->mYd * 0.9,
                        dolphin->mZd * 0.9);
    if (--mCooldown >= 1)
        return;
    mCooldown = 10;
    double distance = dolphin->distanceTo(mTarget);
    if (mState == 0 && distance < (double)mSearchDistance)
        mState = 1;
    else if (mState == 1 && distance > (double)mMaxDistance)
        mState = 0;
}

std::wstring FollowBoatGoal::getName()
{
    return std::wstring(L"Follow_Boat");
}
