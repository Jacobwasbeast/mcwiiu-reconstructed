#include "game/entity/ai/ShulkerDefenseAttackGoalPredicate.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"

bool ShulkerDefenseAttackGoalPredicate::apply(boost::shared_ptr<Entity> entity) const
{
    return (entity->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY
        && (entity->GetType() & ENTITY_TYPE_MONSTER) == ENTITY_TYPE_MONSTER;
}

ShulkerDefenseAttackGoalPredicate::~ShulkerDefenseAttackGoalPredicate()
{
}
