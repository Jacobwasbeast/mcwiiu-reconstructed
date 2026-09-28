#include "game/world/generation/CanAttackPlayerPredicate.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/ai/TargetGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/generation/MobNearestPlayerTargetGoal.hpp"
#include "game/world/generation/Player.hpp"

CanAttackPlayerPredicate::CanAttackPlayerPredicate(MobNearestPlayerTargetGoal* goal)
    : mGoal(goal)
{
}

bool CanAttackPlayerPredicate::apply(boost::shared_ptr<Entity> entity) const
{
    if ((entity->GetType() & EntityType::PLAYER) != EntityType::PLAYER)
    {
        return false;
    }

    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
    if (player->mAbilities.mInvulnerable)
    {
        return false;
    }

    double followDistance = mGoal->getFollowDistance();

    if (entity->isSneaking())
    {
        followDistance *= 0.8f;
    }

    if (entity->isInvisible())
    {
        float armorCoverage = boost::dynamic_pointer_cast<Player>(entity)->getArmorCoverPercentage();
        if (armorCoverage < 0.1f)
        {
            armorCoverage = 0.1f;
        }

        followDistance *= (double)(0.7f * armorCoverage);
    }

    if (entity->distanceTo(mGoal->mMob->shared_from_this()) > followDistance)
    {
        return false;
    }

    return TargetGoal::canAttack(mGoal->mMob, boost::dynamic_pointer_cast<LivingEntity>(entity),
                                 false, true);
}

