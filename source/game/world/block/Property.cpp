#include "game/world/block/Property.hpp"

void Property::updateCachedHashCode()
{
    mCachedHashCode = hashCode();
}
