#include "game/world/generation/LargeFeature.hpp"

#include "game/world/Level.hpp"

LargeFeature::LargeFeature()
{
    range = 8;
    random = Random();
}

LargeFeature::~LargeFeature()
{
}

void LargeFeature::apply(Level* level, int chunkX, int chunkZ, ChunkPrimer* primer)
{
    this->level = level;

    const int featureRange = range;

    random.setSeed(level->getSeed());

    const long long xSeedFactor = random.nextLong();
    const long long zSeedFactor = random.nextLong();

    for (int x = chunkX - featureRange; x <= chunkX + featureRange; ++x)
    {
        for (int z = chunkZ - featureRange; z <= chunkZ + featureRange; ++z)
        {
            random.setSeed(xSeedFactor * x ^ zSeedFactor * z ^ level->getSeed());
            addFeature(level, x, z, chunkX, chunkZ, primer);
        }
    }
}

void LargeFeature::addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ, ChunkPrimer* primer)
{
}
