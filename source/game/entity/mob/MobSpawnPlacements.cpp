#include "game/entity/mob/MobSpawnPlacements.hpp"

#include <boost/unordered_map.hpp>

static boost::unordered_map<unsigned long long, Mob::SpawnPlacementType> spawnPlacements;

void placementTypeCtor__Fv()
{
    spawnPlacements[0x1D000101ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x0D680004ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x2D6A0000ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x2D690001ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x2D690002ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x2D690003ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x2D698001ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x2D698002ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x2D6A0001ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x2D6C0001ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x2D680003ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x2D680005ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x2D680002ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x2D6C0003ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x2D580001ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x0D502001ULL] = Mob::SPAWN_PLACEMENT_TYPE_3;
    spawnPlacements[0x2D680007ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x0D502002ULL] = Mob::SPAWN_PLACEMENT_TYPE_3;
    spawnPlacements[0x0D502003ULL] = Mob::SPAWN_PLACEMENT_TYPE_3;
    spawnPlacements[0x0D502004ULL] = Mob::SPAWN_PLACEMENT_TYPE_3;
    spawnPlacements[0x0D502005ULL] = Mob::SPAWN_PLACEMENT_TYPE_3;
    spawnPlacements[0x0D580002ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x0D6C0002ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x0D600001ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x0D000005ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D540047ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D540045ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D560041ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D540041ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D540043ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x4D54004BULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D0000C1ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D108000ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D540042ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D108001ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D550041ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D540044ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D540441ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D540442ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D540443ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D540046ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D560040ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D550040ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D550042ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D550043ULL] = Mob::SPAWN_PLACEMENT_TYPE_DEFAULT;
    spawnPlacements[0x5D550044ULL] = Mob::SPAWN_PLACEMENT_TYPE_4;
    spawnPlacements[0x5D544000ULL] = Mob::SPAWN_PLACEMENT_TYPE_2;
    spawnPlacements[0x0D602001ULL] = Mob::SPAWN_PLACEMENT_TYPE_3;
}

Mob::SpawnPlacementType MobSpawnPlacements::getPlacementType(unsigned long long entityType)
{
    return spawnPlacements[entityType];
}
