#include "game/core/CanAttackPredicate.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/ai/MobNearestMobTargetGoal.hpp"
#include "game/entity/ai/TargetGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"

CanAttackPredicate::CanAttackPredicate(MobNearestMobTargetGoal* goal)
    : mGoal(goal)
{
}

bool CanAttackPredicate::apply(boost::shared_ptr<Entity> entity) const
{
    if ((entity->GetType() & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
    {
        return false;
    }

    double followDistance = mGoal->getFollowDistance();
    boost::shared_ptr<LivingEntity> livingEntity = boost::static_pointer_cast<LivingEntity>(entity);

    if (livingEntity->isBaby())
    {
        followDistance *= 0.800000011920929;
    }

    if (livingEntity->isInvisible())
    {
        return false;
    }

    if (entity->distanceTo(mGoal->mMob->shared_from_this()) > followDistance)
    {
        return false;
    }

    return TargetGoal::canAttack(mGoal->mMob, livingEntity, false, true);
}

Predicate<boost::shared_ptr<Entity> >* CanAttackPredicate::copy() const
{
    return new CanAttackPredicate(mGoal);
}

CanAttackPredicate::~CanAttackPredicate()
{
}
