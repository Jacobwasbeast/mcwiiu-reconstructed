#include "game/world/biome/RiverBiome.hpp"

#include "game/world/biome/BiomeDecorator.hpp"

static const int RIVER_SEAGRASS_PER_CHUNK = 12;

RiverBiome::RiverBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_River, properties)
{
    mCreatures.clear();
    mChickens.clear();
    mWolves.clear();

    mWaterCreatures.push_back(new MobSpawnerData(16, 0x0D502004ULL, 3, 5));
    mMonsters.push_back(new MobSpawnerData(100, 0x5D550044ULL, 1, 1));

    mDecorator->grassPerChunk = RIVER_SEAGRASS_PER_CHUNK;
}

RiverBiome::~RiverBiome()
{
}

Biome::EBiomeBaseClass RiverBiome::getBaseClass()
{
    return eBiomeBaseClass_River;
}
