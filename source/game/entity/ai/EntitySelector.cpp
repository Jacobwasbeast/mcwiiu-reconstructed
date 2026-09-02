#include "game/entity/ai/EntitySelector.hpp"

#include "game/entity/ai/RidingSelector.hpp"
#include "game/entity/ai/WithinDistanceSelector.hpp"
#include "game/entity/ai/NoSpectatorsSelector.hpp"

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

Predicate<boost::shared_ptr<Entity> >* notRiding(boost::shared_ptr<Entity> entity)
{
    return new RidingSelector(entity, false);
}

}
