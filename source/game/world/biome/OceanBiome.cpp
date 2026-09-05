#include "game/world/biome/OceanBiome.hpp"

OceanBiome::OceanBiome(EBiomeIDs biomeId, bool deep, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Ocean, properties),
      mDeep(deep)
{
    clearDefaultFriendlies();

    mCreatures.push_back(new MobSpawnerData(26, 0x0D502004ULL, 3, 5));
    mCreatures.push_back(new MobSpawnerData(75, 0x0D502002ULL, 4, 7));
    mWaterCreatures.push_back(new MobSpawnerData(10, 0x0D502001ULL, 2, 4));
    mCreatures.push_back(new MobSpawnerData(7, 0x0D602001ULL, 3, 5));
    mMonsters.push_back(new MobSpawnerData(10, 0x5D550044ULL, 2, 4));

}

OceanBiome::~OceanBiome()
{
}

float OceanBiome::getCreatureProbability()
{
    return 0.1F;
}

Biome::EBiomeBaseClass OceanBiome::getBaseClass()
{
    return eBiomeBaseClass_Ocean;
}

Biome::TemperatureCategory OceanBiome::getTemperatureCategory() const
{
    return eTemperatureCategory_Ocean;
}
