#ifndef GAME_WORLD_BLOCK_ENTITYPROPERTYONFIRE_HPP
#define GAME_WORLD_BLOCK_ENTITYPROPERTYONFIRE_HPP

#include "game/world/block/EntityProperty.hpp"

class EntityPropertyOnFire : public EntityProperty
{
public:
    EntityPropertyOnFire(bool onFire);

    virtual bool applies(Random* random, const boost::shared_ptr<Entity>& entity);

private:
    bool mOnFire;
};

#endif
