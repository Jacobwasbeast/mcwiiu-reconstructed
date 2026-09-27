#include "game/entity/ai/FollowBoatGoal.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <vector>

#include "game/core/Boat.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"

FollowBoatGoal::FollowBoatGoal(Dolphin* dolphin)
    : mSearchDistance(4), mMaxDistance(12), mCooldown(0), mTarget(), mState(0),
      mDolphin(dolphin)
{
}

FollowBoatGoal::~FollowBoatGoal()
{
}

bool FollowBoatGoal::canUse()
{
    Entity* dolphin = (Entity*)mDolphin;
    AABB* searchBox = dolphin->mBoundingBox->grow(5.0);
    std::vector<boost::shared_ptr<Entity> >* boats =
        dolphin->mLevel->getEntitiesOfClass(typeid(Boat), searchBox);
    bool found = false;
    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = boats->begin(); it != boats->end(); ++it)
    {
        boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(*it);
        if (boat->getControllingPassenger())
        {
            boost::shared_ptr<Entity> passenger = boat->getControllingPassenger();
            if ((passenger->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
            {
                boost::shared_ptr<LivingEntity> livingPassenger =
                    boost::static_pointer_cast<LivingEntity>(boat->getControllingPassenger());
                if (Mth::abs((float)livingPassenger->mXd) > 0.0f ||
                    Mth::abs((float)livingPassenger->mZd) > 0.0f)
                {
                    found = true;
                    break;
                }
            }
        }
    }
    delete boats;
    return mTarget.expired() && found;
}

bool FollowBoatGoal::canContinueToUse()
{
    boost::shared_ptr<Entity> target = mTarget.lock();
    if (!target || !target->isPassenger())
        return false;
    return Mth::abs((float)target->mXd) > 0.0f ||
           Mth::abs((float)target->mZd) > 0.0f;
}

bool FollowBoatGoal::canInterrupt()
{
    return true;
}

void FollowBoatGoal::start()
{
    Entity* dolphin = (Entity*)mDolphin;
    AABB* searchBox = dolphin->mBoundingBox->grow(5.0);
    std::vector<boost::shared_ptr<Entity> >* boats =
        dolphin->mLevel->getEntitiesOfClass(typeid(Boat), searchBox);
    std::vector<boost::shared_ptr<Entity> >::iterator it;
    boost::shared_ptr<Boat> boat;
    for (it = boats->begin(); it != boats->end(); ++it)
    {
        boat = boost::static_pointer_cast<Boat>(*it);
        if (boat->getControllingPassenger())
        {
            boost::shared_ptr<Entity> passenger = boat->getControllingPassenger();
            if ((passenger->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
                break;
        }
    }
    if (it != boats->end())
        mTarget = boost::static_pointer_cast<LivingEntity>(boat->getControllingPassenger());
    delete boats;
    mCooldown = 0;
    mState = 0;
}

void FollowBoatGoal::stop()
{
    mState = 0;
    mTarget.reset();
}

void FollowBoatGoal::tick()
{
    Entity* dolphin = (Entity*)mDolphin;
    boost::shared_ptr<Entity> target = mTarget.lock();
    if (!target)
        return;
    bool moving = Mth::abs((float)target->mXd) > 0.0f ||
                  Mth::abs((float)target->mZd) > 0.0f;
    float speed = mState == 1 && moving ? 0.2f : 0.15f;
    dolphin->moveRelative((float)dolphin->mXd, (float)dolphin->mYd,
                          (float)dolphin->mZd, speed);
    dolphin->move(eMoverType_Self, dolphin->mXd, dolphin->mYd, dolphin->mZd, false);
    dolphin->mXd *= 0.9;
    dolphin->mYd *= 0.9;
    dolphin->mZd *= 0.9;
    if (--mCooldown >= 1)
        return;
    mCooldown = 10;
    double distance = dolphin->distanceTo(target);
    if (mState == 0 && distance < (double)mSearchDistance)
    {
        mCooldown = 0;
        mState = 1;
    }
    else if (mState == 1 && (double)mMaxDistance < distance)
    {
        mCooldown = 0;
        mState = 0;
    }
}

std::wstring FollowBoatGoal::getName()
{
    return std::wstring(L"Follow Boat");
}
