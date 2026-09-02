#include <cstddef>

#include "game/entity/mob/MobCategory.hpp"
#include "game/world/block/Material.hpp"

static const int NUM_MOB_CATEGORIES = 18;

struct MobSpawnLimits
{
    unsigned long long entityType;
    int maxInstancesPerLevel;
    int extraInstancesForBreeding;
    int extraInstancesForSpawnEgg;
};

static MobSpawnLimits SPAWN_LIMITS[NUM_MOB_CATEGORIES] =
{
    { 0x40ULL,        50,  0, 20 },
    { 0x20000000ULL,  50, 20, 20 },
    { 0x0D000100ULL,  20,  0,  8 },
    { 0x0D502000ULL,  20,  0,  8 },
    { 0x0D502001ULL,   8,  0,  6 },
    { 0x0D6C0002ULL,   8,  8, 10 },
    { 0x0D680004ULL,   8,  8, 10 },
    { 0x2D6A0001ULL,   2, 10,  8 },
    { 0x5D108000ULL,  25,  0,  0 },
    { 0x5D544000ULL,  35,  0,  0 },
    { 0x0D600001ULL,  35,  0, 15 },
    { 0x2D580001ULL,  16,  0,  0 },
    { 0x0D580002ULL,  16,  0,  0 },
    { 0x5D540047ULL,   5,  0,  0 },
    { 0x0C000001ULL,  16,  0,  0 },
    { 0x0800000EULL,  64,  0,  0 },
    { 0x0D602001ULL,  15,  5,  5 },
    { 0x5D540049ULL,   5,  0,  0 }
};

MobCategory* MobCategory::MONSTER;
MobCategory* MobCategory::CREATURE;
MobCategory* MobCategory::AMBIENT;
MobCategory* MobCategory::WATER_CREATURE;
MobCategory* MobCategory::SQUID;
MobCategory* MobCategory::WOLF;
MobCategory* MobCategory::CHICKEN;
MobCategory* MobCategory::MUSHROOM_COW;
MobCategory* MobCategory::SLIME;
MobCategory* MobCategory::GUARDIAN;
MobCategory* MobCategory::VILLAGER;
MobCategory* MobCategory::SNOW_MAN;
MobCategory* MobCategory::VILLAGER_GOLEM;
MobCategory* MobCategory::WITHER_BOSS;
MobCategory* MobCategory::ARMOR_STAND;
MobCategory* MobCategory::ENDER_CRYSTAL;
MobCategory* MobCategory::DOLPHIN;
MobCategory* MobCategory::PHANTOM;

int MobCategory::TOTAL_MAX_INSTANCES;

arrayWithLength<MobCategory*> MobCategory::CATEGORIES(NUM_MOB_CATEGORIES, true);

MobCategory::MobCategory(int maxInstancesPerChunk, Material* spawnPositionMaterial, bool friendly,
                         bool persistent, unsigned long long enumBaseClass, bool singleType)
{
    this->maxInstancesPerChunk = maxInstancesPerChunk;
    this->spawnPositionMaterial = spawnPositionMaterial;
    this->friendly = friendly;
    this->persistent = persistent;
    this->singleType = singleType;
    this->enumBaseClass = enumBaseClass;

    for (int i = 0; i < NUM_MOB_CATEGORIES; i++)
    {
        if (SPAWN_LIMITS[i].entityType == enumBaseClass)
        {
            this->maxInstancesPerLevel = SPAWN_LIMITS[i].maxInstancesPerLevel;
            this->maxInstancesPerLevelWithBreeding = this->maxInstancesPerLevel + SPAWN_LIMITS[i].extraInstancesForBreeding;
            this->maxInstancesPerLevelWithSpawnEgg = this->maxInstancesPerLevelWithBreeding + SPAWN_LIMITS[i].extraInstancesForSpawnEgg;
        }
    }
}

void MobCategory::staticCtor()
{
    MONSTER = new MobCategory(70, Material::AIR, false, false, 0x40ULL, false);
    CREATURE = new MobCategory(10, Material::AIR, true, true, 0x20000000ULL, false);
    AMBIENT = new MobCategory(15, Material::AIR, true, false, 0x0D000100ULL, false);
    WATER_CREATURE = new MobCategory(5, Material::WATER, true, false, 0x0D502000ULL, false);

    CATEGORIES.data[0] = MONSTER;
    CATEGORIES.data[1] = CREATURE;
    CATEGORIES.data[2] = AMBIENT;
    CATEGORIES.data[3] = WATER_CREATURE;

    SQUID = new MobCategory(4, Material::WATER, true, false, 0x0D502001ULL, true);
    WOLF = new MobCategory(3, Material::AIR, true, true, 0x0D6C0002ULL, true);
    CHICKEN = new MobCategory(2, Material::AIR, true, true, 0x0D680004ULL, true);
    MUSHROOM_COW = new MobCategory(2, Material::AIR, true, true, 0x2D6A0001ULL, true);
    SLIME = new MobCategory(15, Material::AIR, false, false, 0x5D108000ULL, false);
    GUARDIAN = new MobCategory(25, Material::WATER, false, false, 0x5D544000ULL, false);

    CATEGORIES.data[4] = SQUID;
    CATEGORIES.data[5] = WOLF;
    CATEGORIES.data[6] = CHICKEN;
    CATEGORIES.data[7] = MUSHROOM_COW;
    CATEGORIES.data[8] = SLIME;
    CATEGORIES.data[9] = GUARDIAN;

    VILLAGER = new MobCategory(25, Material::AIR, false, false, 0x0D600001ULL, true);
    CATEGORIES.data[10] = VILLAGER;

    SNOW_MAN = new MobCategory(25, Material::AIR, false, false, 0x2D580001ULL, true);
    CATEGORIES.data[11] = SNOW_MAN;

    VILLAGER_GOLEM = new MobCategory(25, Material::AIR, false, false, 0x0D580002ULL, true);
    CATEGORIES.data[12] = VILLAGER_GOLEM;

    WITHER_BOSS = new MobCategory(25, Material::AIR, false, false, 0x5D540047ULL, true);
    CATEGORIES.data[13] = WITHER_BOSS;

    ARMOR_STAND = new MobCategory(25, Material::AIR, false, false, 0x0C000001ULL, true);
    CATEGORIES.data[14] = ARMOR_STAND;

    ENDER_CRYSTAL = new MobCategory(25, Material::AIR, false, false, 0x0800000EULL, true);
    CATEGORIES.data[15] = ENDER_CRYSTAL;

    DOLPHIN = new MobCategory(25, Material::WATER, false, false, 0x0D602001ULL, true);
    CATEGORIES.data[16] = DOLPHIN;

    PHANTOM = new MobCategory(5, Material::AIR, false, false, 0x5D540049ULL, true);
    CATEGORIES.data[17] = PHANTOM;

    for (int i = 0; i < NUM_MOB_CATEGORIES; i++)
    {
        TOTAL_MAX_INSTANCES += CATEGORIES.data[i]->getMaxInstancesPerLevelWithSpawnEgg();
    }
}

MobCategory* MobCategory::getCategory(unsigned long long enumBaseClass)
{
    if (enumBaseClass == 0x5D540049ULL)
    {
        return PHANTOM;
    }

    if (enumBaseClass == 0x0D602001ULL)
    {
        return DOLPHIN;
    }

    if (enumBaseClass == 0x0800000EULL)
    {
        return ENDER_CRYSTAL;
    }

    if (enumBaseClass == 0x0C000001ULL)
    {
        return ARMOR_STAND;
    }

    if (enumBaseClass == 0x5D540047ULL)
    {
        return WITHER_BOSS;
    }

    if (enumBaseClass == 0x0D580002ULL)
    {
        return VILLAGER_GOLEM;
    }

    if (enumBaseClass == 0x2D580001ULL)
    {
        return SNOW_MAN;
    }

    if (enumBaseClass == 0x2D6A0001ULL)
    {
        return MUSHROOM_COW;
    }

    if (enumBaseClass == 0x0D6C0002ULL)
    {
        return WOLF;
    }

    if (enumBaseClass == 0x0D502001ULL)
    {
        return SQUID;
    }

    if (enumBaseClass == 0x0D680004ULL)
    {
        return CHICKEN;
    }

    if (enumBaseClass == 0x0D600001ULL)
    {
        return VILLAGER;
    }

    if ((enumBaseClass & 0x20000000ULL) == 0x20000000ULL)
    {
        return CREATURE;
    }

    if ((enumBaseClass & 0x40ULL) == 0x40ULL)
    {
        return MONSTER;
    }

    if ((enumBaseClass & 0x5D108000ULL) == 0x5D108000ULL)
    {
        return SLIME;
    }

    if ((enumBaseClass & 0x5D544000ULL) == 0x5D544000ULL)
    {
        return GUARDIAN;
    }

    if ((enumBaseClass & 0x40000000ULL) == 0x40000000ULL)
    {
        return MONSTER;
    }

    if ((enumBaseClass & 0x0D000100ULL) == 0x0D000100ULL)
    {
        return AMBIENT;
    }

    if ((enumBaseClass & 0x0D502000ULL) == 0x0D502000ULL)
    {
        return WATER_CREATURE;
    }

    return NULL;
}

int MobCategory::getMaxInstancesPerLevel()
{
    return maxInstancesPerLevel;
}

int MobCategory::getMaxInstancesPerLevelWithBreeding()
{
    return maxInstancesPerLevelWithBreeding;
}

int MobCategory::getMaxInstancesPerLevelWithSpawnEgg()
{
    return maxInstancesPerLevelWithSpawnEgg;
}

unsigned long long MobCategory::getEnumBaseClass()
{
    return enumBaseClass;
}

bool MobCategory::isFriendly()
{
    return friendly;
}

bool MobCategory::isPersistent()
{
    return persistent;
}

bool MobCategory::isSingleType()
{
    return singleType;
}
