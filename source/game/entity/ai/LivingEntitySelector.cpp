#include "game/entity/ai/LivingEntitySelector.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"

bool LivingEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    if ((entity->GetType() & ENTITY_TYPE_LIVING_ENTITY) != ENTITY_TYPE_LIVING_ENTITY)
    {
        return false;
    }

    return boost::static_pointer_cast<LivingEntity>(entity)->getMobType() != MOB_TYPE_UNDEAD
        && boost::static_pointer_cast<LivingEntity>(entity)->attackable();
}

LivingEntitySelector::~LivingEntitySelector()
{
}
