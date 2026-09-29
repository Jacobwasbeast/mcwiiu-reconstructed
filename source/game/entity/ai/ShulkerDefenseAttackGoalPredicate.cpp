#include "game/entity/ai/ShulkerDefenseAttackGoalPredicate.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"

bool ShulkerDefenseAttackGoalPredicate::apply(boost::shared_ptr<Entity> entity) const
{
    return (entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY
        && (entity->GetType() & EntityType::ENEMY) == EntityType::ENEMY;
}

ShulkerDefenseAttackGoalPredicate::~ShulkerDefenseAttackGoalPredicate()
{
}
