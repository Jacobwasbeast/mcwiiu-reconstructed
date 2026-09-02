#include "game/entity/ai/ParentHorseSelector.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/Entity.hpp"

bool ParentHorseSelector::apply(boost::shared_ptr<Entity> entity) const
{
    return entity->GetType() == ENTITY_TYPE_HORSE
        && boost::static_pointer_cast<AbstractHorse>(entity)->isBred();
}

ParentHorseSelector::~ParentHorseSelector()
{
}
