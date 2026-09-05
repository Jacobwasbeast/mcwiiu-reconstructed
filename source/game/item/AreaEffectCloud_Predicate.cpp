#include "game/item/AreaEffectCloud_Predicate.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/AreaEffectCloud.hpp"

AreaEffectCloud_Predicate::~AreaEffectCloud_Predicate()
{
}

bool AreaEffectCloud_Predicate::apply(boost::shared_ptr<Entity> entity) const
{
    boost::shared_ptr<AreaEffectCloud> cloud = boost::dynamic_pointer_cast<AreaEffectCloud>(entity);

    return cloud && cloud->isAlive() && cloud->getOwner()
        && cloud->getOwner()->GetType() == ENTITY_TYPE_ENDER_DRAGON;
}
