#include "game/entity/ai/BabyOnLandSelector.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"

bool BabyOnLandSelector::apply(boost::shared_ptr<Entity> entity) const
{
    if ((entity->GetType() & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
    {
        return false;
    }

    boost::shared_ptr<LivingEntity> livingEntity = boost::static_pointer_cast<LivingEntity>(entity);

    return livingEntity->isBaby() && !entity->isInWater();
}

BabyOnLandSelector::~BabyOnLandSelector()
{
}
