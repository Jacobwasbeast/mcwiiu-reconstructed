#include "game/world/biome/OceanColdBiome.hpp"

OceanColdBiome::OceanColdBiome(EBiomeIDs id, bool deep, BiomeProperties* properties)
    : Biome(id, eBiomeType_Ocean, properties)
{
    clearDefaultFriendlies();
    mCreatures.push_back(new MobSpawnerData(26, 0xd502004ULL, 3, 5));
    mCreatures.push_back(new MobSpawnerData(75, 0xd502002ULL, 4, 7));
    mWaterCreatures.push_back(new MobSpawnerData(10, 0xd502001ULL, 2, 4));
    mCreatures.push_back(new MobSpawnerData(7, 0xd602001ULL, 3, 5));
    mSquid.push_back(new MobSpawnerData(10, 0x5d550044ULL, 2, 4));
    mRunDepth = 0;
    mSnowAccumulation = 1.0f;
    mSnowMeltRate = 0.1f;
    mMinSnowAccumulation = deep ? 0.5f : 0.1f;
    mMaxSnowAccumulation = deep ? 0.95f : 0.9f;
}

OceanColdBiome::~OceanColdBiome()
{
}

float OceanColdBiome::getCreatureProbability()
{
    return 0.1f;
}

Biome::EBiomeBaseClass OceanColdBiome::getBaseClass()
{
    return eBiomeBaseClass_OceanCold;
}

Biome::TemperatureCategory OceanColdBiome::getTemperatureCategory() const
{
    return eTemperatureCategory_Ocean;
}
