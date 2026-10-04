#include "game/entity/ai/ParentHorseSelector.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/Entity.hpp"

bool ParentHorseSelector::apply(boost::shared_ptr<Entity> entity) const
{
    const bool isHorse = entity->GetType() == EntityType::HORSE;
    return isHorse && boost::static_pointer_cast<AbstractHorse>(entity)->isBred();
}

ParentHorseSelector::~ParentHorseSelector()
{
}
