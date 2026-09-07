#include "game/entity/ai/EntitySelector.hpp"

#include "game/entity/ai/RidingSelector.hpp"
#include "game/entity/ai/WithinDistanceSelector.hpp"
#include "game/entity/ai/NoSpectatorsSelector.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"

bool validPlayerPredicate(boost::shared_ptr<Player> player)
{
    static Predicate<boost::shared_ptr<Entity> >* validPlayerDistance = NULL;

    if (validPlayerDistance == NULL)
    {
        validPlayerDistance = EntitySelector::withinDistance(0.0, 128.0, 0.0, 192.0);
    }

    return EntitySelector::NO_SPECTATORS->apply(player)
        && validPlayerDistance->apply(player);
}

bool visibleEnemySelectorFunc(boost::shared_ptr<Entity> entity)
{
    return entity->isAlive() && !entity->isInvisible();
}

namespace EntitySelector
{
    Predicate<boost::shared_ptr<Entity> >* NO_SPECTATORS = new NoSpectatorsSelector();
}

namespace EntitySelector
{

Predicate<boost::shared_ptr<Entity> >* withinDistance(double x, double y, double z, double distance)
{
    return new WithinDistanceSelector(x, y, z, distance);
}

Predicate<boost::shared_ptr<Entity> >* riding(boost::shared_ptr<Entity> entity)
{
    return new RidingSelector(entity, true);
}

Predicate<boost::shared_ptr<Entity> >* notRiding(boost::shared_ptr<Entity> entity)
{
    return new RidingSelector(entity, false);
}

}
