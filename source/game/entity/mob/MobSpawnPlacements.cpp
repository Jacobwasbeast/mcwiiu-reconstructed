#include "game/entity/mob/MobSpawnPlacements.hpp"

#include <boost/unordered_map.hpp>

static boost::unordered_map<unsigned long long, Mob::SpawnPlacementType> spawnPlacements;

Mob::SpawnPlacementType MobSpawnPlacements::getPlacementType(unsigned long long enumBaseClass)
{
    return spawnPlacements[enumBaseClass];
}
