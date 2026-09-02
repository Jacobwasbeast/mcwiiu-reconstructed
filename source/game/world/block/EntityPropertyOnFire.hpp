#ifndef GAME_WORLD_BLOCK_ENTITYPROPERTYONFIRE_HPP
#define GAME_WORLD_BLOCK_ENTITYPROPERTYONFIRE_HPP

#include <boost/shared_ptr.hpp>

class Entity;
class Random;

class EntityPropertyOnFire
{
public:
    EntityPropertyOnFire(bool onFire);

    virtual bool applies(Random* random, const boost::shared_ptr<Entity>& entity);

private:
    bool mOnFire;
};

#endif
