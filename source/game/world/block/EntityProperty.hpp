#ifndef GAME_WORLD_BLOCK_ENTITYPROPERTY_HPP
#define GAME_WORLD_BLOCK_ENTITYPROPERTY_HPP

#include <boost/shared_ptr.hpp>

#include "game/util/Random.hpp"

class Entity;

class EntityProperty
{
public:
    virtual bool applies(Random* random, const boost::shared_ptr<Entity>& entity) = 0;
};

#endif
