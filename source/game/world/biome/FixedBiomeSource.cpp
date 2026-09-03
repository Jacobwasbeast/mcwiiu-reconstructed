#include "game/world/biome/FixedBiomeSource.hpp"

#include <algorithm>

#include "game/util/Random.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockPos.hpp"

void MemSect(int section);

FixedBiomeSource::FixedBiomeSource(Biome* biome, float rainfall)
{
    mBiome = biome;
}

FixedBiomeSource::~FixedBiomeSource()
{
}

Biome* FixedBiomeSource::getBiome(const BlockPos& pos)
{
    return mBiome;
}

void FixedBiomeSource::getRawBiomeBlock(arrayWithLength<Biome*>& biomes, int x, int z, int width, int height) const
{
    int count = width * height;

    MemSect(36);

    biomes.resize(count);

    MemSect(0);

    std::fill(biomes.data, biomes.data + count, mBiome);
}

void FixedBiomeSource::getBiomeBlock(arrayWithLength<Biome*>& biomes, int x, int z, int width, int height, bool useCache) const
{
    int count = width * height;

    MemSect(36);

    biomes.resize(count);

    MemSect(0);

    std::fill(biomes.data, biomes.data + count, mBiome);
}

void FixedBiomeSource::getBiomeIndexBlock(arrayWithLength<unsigned char>& biomeIndices,
                                          int x,
                                          int z,
                                          int width,
                                          int height,
                                          bool useCache) const
{
    int count = width * height;

    MemSect(36);

    biomeIndices.resize(count);

    MemSect(0);

    std::fill(biomeIndices.data, biomeIndices.data + count, (unsigned char)mBiome->id);
}

bool FixedBiomeSource::containsOnly(int x, int z, int radius, const std::vector<Biome*>& biomes)
{
    return std::find(biomes.begin(), biomes.end(), mBiome) != biomes.end();
}

BlockPos* FixedBiomeSource::findBiome(int x, int z, int radius, std::vector<Biome*> biomes, Random* random)
{
    if (std::find(biomes.begin(), biomes.end(), mBiome) == biomes.end())
    {
        return NULL;
    }

    int offsetX = random->nextInt(radius * 2 + 1);
    int offsetZ = random->nextInt(radius * 2 + 1);

    return new BlockPos((x - radius) + offsetX, 0, (z - radius) + offsetZ);
}

bool FixedBiomeSource::isFixedBiome()
{
    return true;
}

Biome* FixedBiomeSource::getFixedBiome()
{
    return mBiome;
}
