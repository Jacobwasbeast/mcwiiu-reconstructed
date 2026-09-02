#include "game/entity/ai/PreySelector.hpp"

#include "game/entity/mob/Entity.hpp"

bool PreySelector::apply(boost::shared_ptr<Entity> entity) const
{
    return entity->GetType() == ENTITY_TYPE_SHEEP || entity->GetType() == ENTITY_TYPE_RABBIT;
}

PreySelector::~PreySelector()
{
}
