#include "game/world/generation/CanAttackPlayerPredicate.hpp"

#include "game/core/AttributeInstance.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MobNearestPlayerTargetGoal.hpp"
#include "game/entity/ai/TargetGoal.hpp"

CanAttackPlayerPredicate::CanAttackPlayerPredicate(MobNearestPlayerTargetGoal* goal)
    : mGoal(goal)
{
}

bool CanAttackPlayerPredicate::apply(boost::shared_ptr<Entity> entity) const
{
    if ((entity->GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER)
        return false;

    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
    if (!player || player->mAbilities.mInvulnerable)
        return false;

    double followDistance = mGoal->getFollowDistance();
    if (entity->isSneaking())
        followDistance *= 0.8;

    if (entity->isInvisible())
    {
        boost::shared_ptr<LivingEntity> livingEntity = boost::dynamic_pointer_cast<LivingEntity>(entity);
        double attributeValue = livingEntity->getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->getValue();
        if (attributeValue < 0.1)
            attributeValue = 0.1;
        followDistance *= (float)(attributeValue * 0.7);
    }

    if (mGoal->mMob->distanceTo(entity) > followDistance)
        return false;

    boost::shared_ptr<LivingEntity> livingEntity = boost::dynamic_pointer_cast<LivingEntity>(entity);
    return TargetGoal::canAttack(mGoal->mMob, livingEntity, false, true);
}

CanAttackPlayerPredicate::~CanAttackPlayerPredicate()
{
}
