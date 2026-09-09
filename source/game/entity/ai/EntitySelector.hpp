#ifndef GAME_ENTITY_AI_ENTITYSELECTOR_HPP
#define GAME_ENTITY_AI_ENTITYSELECTOR_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/Predicate.hpp"

class Entity;
class Player;

bool validPlayerPredicate(boost::shared_ptr<Player> player);
bool visibleEnemySelectorFunc(boost::shared_ptr<Entity> entity);

namespace EntitySelector
{
    extern Predicate<boost::shared_ptr<Entity> >* NO_SPECTATORS;
    extern Predicate<boost::shared_ptr<Entity> >* NO_CREATIVE_OR_SPECTATOR;
    extern Predicate<boost::shared_ptr<Entity> >* ENTITY_STILL_ALIVE;

    Predicate<boost::shared_ptr<Entity> >* withinDistance(double x, double y, double z, double distance);
    Predicate<boost::shared_ptr<Entity> >* riding(boost::shared_ptr<Entity> entity);
    Predicate<boost::shared_ptr<Entity> >* notRiding(boost::shared_ptr<Entity> entity);
}

#endif
