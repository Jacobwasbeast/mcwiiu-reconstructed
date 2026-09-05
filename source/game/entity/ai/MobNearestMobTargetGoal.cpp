#include "game/entity/ai/MobNearestMobTargetGoal.hpp"

#include <algorithm>
#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/AttributeInstance.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"

MobNearestMobTargetGoal::MobNearestMobTargetGoal(Mob* mob,
                                                 const std::type_info& targetType)
    : Goal(), mMob(mob), mCanAttack(this), mSorter(mob), mTarget(),
      mTargetType(&targetType)
{
}

MobNearestMobTargetGoal::~MobNearestMobTargetGoal()
{
}

double MobNearestMobTargetGoal::getFollowDistance()
{
    AttributeInstance* attribute = mMob->getAttribute(SharedMonsterAttributes::FOLLOW_RANGE);
    if (attribute == NULL)
        return 16.0;
    return attribute->getValue();
}

bool MobNearestMobTargetGoal::canUse()
{
    double followDistance = getFollowDistance();
    AABB* searchBox = mMob->mBoundingBox->grow(followDistance, 4.0, followDistance);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mMob->mLevel->getEntitiesOfClass(*mTargetType, searchBox, &mCanAttack, true);
    if (entities == NULL || entities->empty())
    {
        delete entities;
        return false;
    }

    std::sort(entities->begin(), entities->end(), mSorter);
    boost::shared_ptr<LivingEntity> target =
        boost::static_pointer_cast<LivingEntity>((*entities)[0]);
    if (mMob->distanceToSqr((*entities)[0]) <= followDistance * followDistance)
        mTarget = target;
    else
        mTarget.reset();
    delete entities;
    return !!mTarget;
}

bool MobNearestMobTargetGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target || !target->isAlive())
        return false;

    double followDistance = getFollowDistance();
    if (mMob->distanceToSqr(target) > followDistance * followDistance)
        return false;

    if (target->GetType() == 0x000000000C800001ULL && target->isCreative())
        return false;

    return true;
}

void MobNearestMobTargetGoal::start()
{
    mMob->setTarget(mTarget);
    Goal::start();
}

void MobNearestMobTargetGoal::stop()
{
    mMob->setTarget(boost::shared_ptr<LivingEntity>());
    Goal::start();
}

std::wstring MobNearestMobTargetGoal::getName()
{
    return std::wstring(L"MobNearestMobTarget");
}
