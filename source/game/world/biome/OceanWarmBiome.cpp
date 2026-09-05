#include "game/world/biome/OceanWarmBiome.hpp"

#include "game/util/Random.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/Level.hpp"

OceanWarmBiome::OceanWarmBiome(EBiomeIDs biomeId, bool deep, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Ocean, properties),
      mDeep(deep)
{
    clearDefaultFriendlies();

    if (!mDeep)
    {
        mCreatures.push_back(new MobSpawnerData(25, 0x0D502003ULL, 3, 5));
        mCreatures.push_back(new MobSpawnerData(75, 0x0D502005ULL, 6, 10));
        mCreatures.push_back(new MobSpawnerData(25, 0x0D502005ULL, 3, 5));
    }

    mWaterCreatures.push_back(new MobSpawnerData(10, 0x0D502001ULL, 2, 4));
    mCreatures.push_back(new MobSpawnerData(7, 0x0D602001ULL, 3, 5));
    mMonsters.push_back(new MobSpawnerData(10, 0x5D550044ULL, 2, 4));
    mRunDepth = 1;
    mSnowAccumulation = 0.3F;
    mSnowMeltRate = 0.9F;
    mSnowEnabled = true;
}

OceanWarmBiome::~OceanWarmBiome()
{
}

float OceanWarmBiome::getCreatureProbability()
{
    return 0.1F;
}

void OceanWarmBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    Biome::decorate(level, random, pos);
}

Biome::EBiomeBaseClass OceanWarmBiome::getBaseClass()
{
    return eBiomeBaseClass_OceanWarm;
}

Biome::TemperatureCategory OceanWarmBiome::getTemperatureCategory() const
{
    return eTemperatureCategory_Ocean;
}
