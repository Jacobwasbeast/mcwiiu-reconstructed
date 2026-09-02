#include "game/entity/ai/RidingSelector.hpp"

#include "game/entity/mob/Entity.hpp"

bool RidingSelector::apply(boost::shared_ptr<Entity> entity) const
{
    if (riding)
    {
        return entity->getVehicle() == vehicle;
    }

    return entity->getVehicle() != vehicle;
}

RidingSelector::~RidingSelector()
{
}
