#include "game/entity/ai/BabyOnLandSelector.hpp"

#include "game/entity/mob/Entity.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/LivingEntity.hpp"

bool BabyOnLandSelector::apply(boost::shared_ptr<Entity> entity) const
{
    if ((entity->GetType() & ENTITY_TYPE_LIVING_ENTITY) != ENTITY_TYPE_LIVING_ENTITY)
    {
        return false;
    }

    boost::shared_ptr<LivingEntity> mob = boost::static_pointer_cast<LivingEntity>(entity);

    return mob->isBaby() && !entity->isInWater();
}

BabyOnLandSelector::~BabyOnLandSelector()
{
}
