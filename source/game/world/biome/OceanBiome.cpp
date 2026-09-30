#include "game/world/biome/OceanBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/world/biome/BiomeDecorator.hpp"

OceanBiome::OceanBiome(EBiomeIDs biomeId, bool deep, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Ocean, properties),
      mDeep(deep)
{
    clearDefaultFriendlies();

    mWaterCreatures.push_back(new MobSpawnerData(26, EntityType::SALMON, 3, 5));
    mWaterCreatures.push_back(new MobSpawnerData(75, EntityType::FISH, 4, 7));
    mSquid.push_back(new MobSpawnerData(10, EntityType::SQUID, 2, 4));
    mWaterCreatures.push_back(new MobSpawnerData(7, EntityType::DOLPHIN, 3, 5));
    mMonsters.push_back(new MobSpawnerData(10, EntityType::DROWNED, 2, 4));

    mOceanRuinConfig.biomeType = OceanRuin::eRuinType_Cold;
    mOceanRuinConfig.smallProbability = mDeep ? 0.5F : 0.3F;
    mOceanRuinConfig.clusterProbability = mDeep ? 0.95F : 0.9F;
    mOceanRuinConfig.enabled = true;

    mDecorator->kelpPerChunk = 20;
    mDecorator->seagrassPerChunk = 12;
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
