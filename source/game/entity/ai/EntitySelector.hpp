#ifndef GAME_ENTITY_AI_ENTITYSELECTOR_HPP
#define GAME_ENTITY_AI_ENTITYSELECTOR_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/Predicate.hpp"

class Entity;

namespace EntitySelector
{
    extern Predicate<boost::shared_ptr<Entity> >* NO_SPECTATORS;

    Predicate<boost::shared_ptr<Entity> >* withinDistance(double x, double y, double z, double distance);
    Predicate<boost::shared_ptr<Entity> >* riding(boost::shared_ptr<Entity> entity);
    Predicate<boost::shared_ptr<Entity> >* notRiding(boost::shared_ptr<Entity> entity);
}

#endif
