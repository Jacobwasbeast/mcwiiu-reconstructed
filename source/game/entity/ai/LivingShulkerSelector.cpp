#include "game/entity/ai/LivingShulkerSelector.hpp"

#include "game/entity/mob/Entity.hpp"

#include <boost/shared_ptr.hpp>

bool LivingShulkerSelector::apply(boost::shared_ptr<Entity> entity) const
{
    return entity->GetType() == ENTITY_TYPE_SHULKER && entity->isAlive();
}

LivingShulkerSelector::~LivingShulkerSelector()
{
}
