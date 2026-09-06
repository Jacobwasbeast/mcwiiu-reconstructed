#include "game/world/biome/OceanLukewarmBiome.hpp"

#include "game/world/biome/BiomeDecorator.hpp"

OceanLukewarmBiome::OceanLukewarmBiome(EBiomeIDs biomeId, bool deep, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Ocean, properties)
{
    clearDefaultFriendlies();

    mCreatures.push_back(new MobSpawnerData(26, 0x0D502004ULL, 3, 5));
    mCreatures.push_back(new MobSpawnerData(75, 0x0D502002ULL, 4, 7));

    if (!deep)
    {
        mCreatures.push_back(new MobSpawnerData(25, 0x0D502003ULL, 3, 5));
        mCreatures.push_back(new MobSpawnerData(75, 0x0D502005ULL, 6, 10));
        mCreatures.push_back(new MobSpawnerData(25, 0x0D502005ULL, 3, 5));
    }

    mWaterCreatures.push_back(new MobSpawnerData(10, 0x0D502001ULL, 2, 4));
    mCreatures.push_back(new MobSpawnerData(7, 0x0D602001ULL, 3, 5));
    mMonsters.push_back(new MobSpawnerData(10, 0x5D550044ULL, 2, 4));

    mOceanRuinConfig.biomeType = OceanRuin::eRuinType_Warm;
    mOceanRuinConfig.smallProbability = 0.3F;
    mOceanRuinConfig.clusterProbability = 0.9F;
    mOceanRuinConfig.enabled = true;

    mDecorator->kelpPerChunk = 20;
    mDecorator->seagrassPerChunk = 12;
    mRunDepth = 1;
    mSnowAccumulation = 0.1F;
    mSnowMeltRate = 0.9F;
    mSnowEnabled = true;
}

OceanLukewarmBiome::~OceanLukewarmBiome()
{
}

float OceanLukewarmBiome::getCreatureProbability()
{
    return 0.1F;
}

Biome::EBiomeBaseClass OceanLukewarmBiome::getBaseClass()
{
    return eBiomeBaseClass_OceanLukewarm;
}

Biome::TemperatureCategory OceanLukewarmBiome::getTemperatureCategory() const
{
    return eTemperatureCategory_Ocean;
}
