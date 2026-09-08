#include "game/world/biome/OceanBiome.hpp"

#include "game/world/biome/BiomeDecorator.hpp"

OceanBiome::OceanBiome(EBiomeIDs biomeId, bool deep, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Ocean, properties),
      mDeep(deep)
{
    clearDefaultFriendlies();

    mWaterCreatures.push_back(new MobSpawnerData(26, 0x0D502004ULL, 3, 5));
    mWaterCreatures.push_back(new MobSpawnerData(75, 0x0D502002ULL, 4, 7));
    mSquid.push_back(new MobSpawnerData(10, 0x0D502001ULL, 2, 4));
    mWaterCreatures.push_back(new MobSpawnerData(7, 0x0D602001ULL, 3, 5));
    mMonsters.push_back(new MobSpawnerData(10, 0x5D550044ULL, 2, 4));

    mOceanRuinConfig.biomeType = OceanRuin::eRuinType_Cold;
    mOceanRuinConfig.smallProbability = mDeep ? 0.5F : 0.3F;
    mOceanRuinConfig.clusterProbability = mDeep ? 0.95F : 0.9F;
    mOceanRuinConfig.enabled = true;

    mDecorator->kelpPerChunk = 20;
    mDecorator->seagrassPerChunk = 12;
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
