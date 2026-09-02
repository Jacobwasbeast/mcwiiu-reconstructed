#include "game/entity/ai/WithinDistanceSelector.hpp"

#include "game/entity/mob/Entity.hpp"

bool WithinDistanceSelector::apply(boost::shared_ptr<Entity> entity) const
{
    if (!entity)
    {
        return false;
    }

    return entity->distanceToSqr(x, y, z) <= distanceSqr;
}

WithinDistanceSelector::~WithinDistanceSelector()
{
}
