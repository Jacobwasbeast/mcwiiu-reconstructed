#include "game/entity/ai/PreySelector.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"

bool PreySelector::apply(boost::shared_ptr<Entity> entity) const
{
    return entity->GetType() == EntityType::SHEEP || entity->GetType() == EntityType::RABBIT;
}

PreySelector::~PreySelector()
{
}
