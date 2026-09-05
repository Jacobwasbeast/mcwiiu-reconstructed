#include "game/world/generation/MobNearestPlayerTargetGoal.hpp"

#include <algorithm>
#include <typeinfo>

#include <boost/shared_ptr.hpp>

#include "game/core/AttributeInstance.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"

MobNearestPlayerTargetGoal::MobNearestPlayerTargetGoal(Mob* mob)
    : Goal(), mMob(mob), mTargetSelector(this), mSorter(mob), mTarget()
{
}

MobNearestPlayerTargetGoal::~MobNearestPlayerTargetGoal()
{
}

double MobNearestPlayerTargetGoal::getFollowDistance()
{
    AttributeInstance* attribute = mMob->getAttribute(SharedMonsterAttributes::FOLLOW_RANGE);
    if (attribute == NULL)
        return 16.0;
    return attribute->getValue();
}

bool MobNearestPlayerTargetGoal::canUse()
{
    AABB* searchArea = mMob->getBoundingBox()->grow(getFollowDistance(), 4.0,
                                                     getFollowDistance());
    std::vector<boost::shared_ptr<Entity> >* entities =
        mMob->mLevel->getEntitiesOfClass(typeid(Entity), searchArea,
                                         &mTargetSelector, false);
    if (entities == NULL || entities->empty())
    {
        delete entities;
        return false;
    }

    std::sort(entities->begin(), entities->end(), mSorter);
    mTarget = boost::dynamic_pointer_cast<LivingEntity>((*entities)[0]);
    delete entities;
    return !!mTarget;
}

bool MobNearestPlayerTargetGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target || !target->isAlive())
        return false;

    if (target != mTarget)
    {
        double distance = getFollowDistance();
        if (mMob->distanceToSqr(target) > distance * distance)
            return false;

        if (target->GetType() == 0x000000000C800001ULL && target->isCreative())
            return false;
    }
    return true;
}

void MobNearestPlayerTargetGoal::start()
{
    mMob->setTarget(mTarget);
    Goal::start();
}

void MobNearestPlayerTargetGoal::stop()
{
    mMob->setTarget(boost::shared_ptr<LivingEntity>());
    Goal::start();
}

std::wstring MobNearestPlayerTargetGoal::getName()
{
    return std::wstring(L"MobNearestPlayerTarget");
}
