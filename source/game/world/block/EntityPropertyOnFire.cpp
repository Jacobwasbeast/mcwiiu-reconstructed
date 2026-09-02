#include "game/world/block/EntityPropertyOnFire.hpp"

#include "game/entity/mob/Entity.hpp"

EntityPropertyOnFire::EntityPropertyOnFire(bool onFire)
    : mOnFire(onFire)
{
}

bool EntityPropertyOnFire::applies(Random* random, const boost::shared_ptr<Entity>& entity)
{
    return entity->isOnFire() == mOnFire;
}
