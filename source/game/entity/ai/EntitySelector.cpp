#include "game/entity/ai/EntitySelector.hpp"

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
