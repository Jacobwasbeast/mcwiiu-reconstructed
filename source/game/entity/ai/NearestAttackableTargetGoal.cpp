#include "game/entity/ai/NearestAttackableTargetGoal.hpp"

#include <algorithm>
#include <typeinfo>
#include <boost/shared_ptr.hpp>

#include "game/entity/ai/SubselectEntitySelector.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Random.hpp"
#include "game/util/AABB.hpp"
#include "game/world/generation/Player.hpp"

NearestAttackableTargetGoal::DistComp::DistComp(Entity* entity)
    : mEntity(entity)
{
}

bool NearestAttackableTargetGoal::DistComp::operator()(boost::shared_ptr<Entity> first,
                                                        boost::shared_ptr<Entity> second)
{
    return mEntity->distanceToSqr(first) < mEntity->distanceToSqr(second);
}

NearestAttackableTargetGoal::DisguisedRangeModifier::DisguisedRangeModifier(PathfinderMob* mob)
    : mMob(mob)
{
}

NearestAttackableTargetGoal::DisguisedRangeModifier::~DisguisedRangeModifier()
{
}

double NearestAttackableTargetGoal::DisguisedRangeModifier::apply(boost::shared_ptr<Player> player) const
{
    not_null_ptr<ItemInstance> item = player->getMainHandItem();
    if (item->getItem() != NULL)
    {
        int damage = item->getDamageValue();
        unsigned long long type = mMob->GetType();
        if ((type == ENTITY_TYPE_PIG_ZOMBIE && damage == 0)
            || (type == ENTITY_TYPE_ZOMBIE && damage == 2)
            || (type == ENTITY_TYPE_ZOMBIE_VILLAGER && damage == 4))
            return 0.5;
    }
    return 1.0;
}

NearestAttackableTargetGoal::NearestAttackableTargetGoal(
    PathfinderMob* mob, const std::type_info& targetType, bool mustSee,
    bool mustReach, bool includeRemoved)
    : TargetGoal(mob, mustSee, mustReach), mTargetType(&targetType), mRandomInterval(10),
      mSorter(mob)
{
    setRequiredControlFlags(1);
    mTargetSelector = new SubselectEntitySelector(this, NULL, false);
    mIncludeRemoved = includeRemoved;
    mRangeModifier = new DisguisedRangeModifier(mob);
}

NearestAttackableTargetGoal::NearestAttackableTargetGoal(
    PathfinderMob* mob, const std::type_info& targetType, int randomInterval,
    bool mustSee, bool mustReach, const Predicate<boost::shared_ptr<Entity> >* selector,
    bool includeRemoved, bool ownsSelector)
    : TargetGoal(mob, mustSee, mustReach), mTargetType(&targetType),
      mRandomInterval(randomInterval), mSorter(mob)
{
    setRequiredControlFlags(1);
    mTargetSelector = new SubselectEntitySelector(this, selector, ownsSelector);
    mIncludeRemoved = includeRemoved;
    mRangeModifier = new DisguisedRangeModifier(mob);
}

NearestAttackableTargetGoal::~NearestAttackableTargetGoal()
{
    delete mRangeModifier;
    delete mTargetSelector;
}

bool NearestAttackableTargetGoal::canUse()
{
    if (mRandomInterval > 0 && mMob->getRandom()->nextInt(mRandomInterval) != 0)
        return false;
    if (*mTargetType == typeid(Player) || *mTargetType == typeid(ServerPlayer))
    {
        mTarget = mMob->mLevel->getNearestAttackablePlayer(
            mMob->mX, mMob->mY, mMob->mZ,
            getFollowDistance() + (double)mMob->mBbWidth,
            (double)mMob->mBbHeight, mRangeModifier, mTargetSelector);
        return !!mTarget;
    }
    std::vector<boost::shared_ptr<Entity> >* entities =
        mMob->mLevel->getEntitiesOfClass(*mTargetType,
            getTargetSearchArea(getFollowDistance()), mTargetSelector, mIncludeRemoved);
    if (entities == NULL || entities->empty())
    {
        delete entities;
        return false;
    }
    std::sort(entities->begin(), entities->end(), mSorter);
    double distance = getFollowDistance();
    if (mMob->distanceToSqr((*entities)[0]) <= distance * distance)
        mTarget = boost::dynamic_pointer_cast<LivingEntity>((*entities)[0]);
    else
        mTarget.reset();
    delete entities;
    return !!mTarget;
}

bool NearestAttackableTargetGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
        target = mTarget;
    if (!target || !target->isAlive())
        return false;
    if (mTargetSelector && !mTargetSelector->apply(target))
        return false;
    return TargetGoal::canContinueToUse();
}

void NearestAttackableTargetGoal::start()
{
    mMob->setTarget(mTarget);
    TargetGoal::start();
}

std::wstring NearestAttackableTargetGoal::getName()
{
    return L"NearestAttackableTarget";
}

AABB* NearestAttackableTargetGoal::getTargetSearchArea(double distance)
{
    return mMob->mBoundingBox->grow(distance, 4.0, distance);
}
