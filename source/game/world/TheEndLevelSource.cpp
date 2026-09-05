#include "game/world/TheEndLevelSource.hpp"

#include <cmath>

#include "game/entity/mob/MobCategory.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/LevelChunk.hpp"

TheEndLevelSource::TheEndLevelSource(Level* level, bool generateStructures, long long seed, const BlockPos& origin)
    : mLevelHeight(128),
      mRandom(new Random(seed)),
      mPopulationRandom(new Random(seed)),
      mNoise1(new PerlinNoise(mRandom, 16)),
      mNoise2(new PerlinNoise(mRandom, 16)),
      mNoise3(new PerlinNoise(mRandom, 8)),
      mNoise4(new PerlinNoise(mRandom, 10)),
      mNoise5(new PerlinNoise(mRandom, 16)),
      mLevel(level),
      mGenerateStructures(generateStructures),
      mOrigin(origin),
      mEndCityFeature(NULL),
      mIslandNoise(new SimplexNoise(mRandom)),
      mNoiseField(),
      mBiomes()
{
}

TheEndLevelSource::~TheEndLevelSource()
{
    delete mRandom;
    delete mPopulationRandom;
    delete mNoise1;
    delete mNoise2;
    delete mNoise3;
    delete mNoise4;
    delete mNoise5;
    delete mIslandNoise;
    delete[] mBiomes.data;
    delete[] mNoiseField.data;
}

double TheEndLevelSource::fjIslandProximity(int x, int z)
{
    const double distance = std::sqrt(static_cast<double>(x * x + z * z));
    return distance < 64.0 ? 12.0 : std::min(std::fabs(static_cast<double>(x)), std::fabs(static_cast<double>(z)));
}

bool TheEndLevelSource::fjIslandCheck(int x, int z)
{
    return fjIslandProximity(x, z) < 3.0;
}

double TheEndLevelSource::getIslandHeightValue(int x, int z, int xOffset, int zOffset)
{
    const int sampleX = x * 2 + xOffset;
    const int sampleZ = z * 2 + zOffset;
    if (!fjIslandCheck(x, z))
    {
        const double distance = std::sqrt(static_cast<double>(sampleX * sampleX + sampleZ * sampleZ));
        return -(distance * 0.25 - 100.0);
    }

    double height = 100.0 - std::sqrt(static_cast<double>(sampleX * sampleX + sampleZ * sampleZ)) * 8.0;
    for (int ix = -4; ix <= 4; ++ix)
    {
        for (int iz = -4; iz <= 4; ++iz)
        {
            const int px = x + ix;
            const int pz = z + iz;
            if (fjIslandCheck(px, pz))
            {
                const double islandDistance = std::sqrt(static_cast<double>((px * 2 + xOffset) * (px * 2 + xOffset) + (pz * 2 + zOffset) * (pz * 2 + zOffset)));
                const double island = 80.0 - islandDistance * (18.0 + static_cast<double>((std::abs(px) + std::abs(pz)) % 6));
                if (island > height)
                {
                    height = island;
                }
            }
        }
    }
    return height;
}

arrayWithLength<double> TheEndLevelSource::getHeights(arrayWithLength<double> values, int x, int y, int z, int xSize, int ySize, int zSize)
{
    const unsigned int count = static_cast<unsigned int>(xSize * ySize * zSize);
    if (values.data == NULL || values.length != count)
    {
        delete[] values.data;
        values.data = count == 0 ? NULL : new double[count];
        values.length = count;
    }

    for (int ix = 0; ix < xSize; ++ix)
    {
        for (int iz = 0; iz < zSize; ++iz)
        {
            for (int iy = 0; iy < ySize; ++iy)
            {
                const double px = static_cast<double>(x + ix) * 0.25;
                const double pz = static_cast<double>(z + iz) * 0.25;
                const double base = getIslandHeightValue(x + ix / 2, z + iz / 2, ix & 1, iz & 1);
                const double noise = mNoise1->getValue(px, pz) * 8.0 + mNoise2->getValue(px * 0.5, pz * 0.5) * 4.0;
                const double vertical = static_cast<double>(y + iy * 4);
                values.data[(ix * zSize + iz) * ySize + iy] = base + noise - vertical;
            }
        }
    }
    return values;
}

void TheEndLevelSource::prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer)
{
    mNoiseField = getHeights(mNoiseField, chunkX * 2, 0, chunkZ * 2, 3, 33, 3);
    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            for (int y = 0; y < 128; ++y)
            {
                const int sampleX = x >> 3;
                const int sampleZ = z >> 3;
                const int sampleY = y >> 2;
                const double value = mNoiseField.data[(sampleX * 3 + sampleZ) * 33 + sampleY];
                primer->setState(x, y, z, value > 0.0 ? Blocks::end_stone->defaultBlockState() : Blocks::AIR->defaultBlockState());
            }
        }
    }
}

void TheEndLevelSource::buildSurfaces(ChunkPrimer* primer)
{
    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            int depth = -1;
            for (int y = 127; y >= 0; --y)
            {
                const BlockState* state = primer->getState(x, y, z);
                if (state->getBlock() == Blocks::AIR)
                {
                    depth = -1;
                }
                else if (depth == -1)
                {
                    depth = 3;
                }
                else if (depth > 0)
                {
                    --depth;
                    primer->setState(x, y, z, Blocks::end_stone->defaultBlockState());
                }
            }
        }
    }
}

LevelChunk* TheEndLevelSource::createChunk(int chunkX, int chunkZ)
{
    mRandom->setSeed(static_cast<long long>(chunkX) * 341873128712LL + static_cast<long long>(chunkZ) * 132897987541LL);
    arrayWithLength<unsigned short> elements(65536, true);
    ChunkPrimer primer(false, elements);
    prepareHeights(chunkX, chunkZ, &primer);
    buildSurfaces(&primer);
    LevelChunk* chunk = new LevelChunk(mLevel, &primer, chunkX, chunkZ);
    mLevel->getBiomeSource()->getBiomeBlock(mBiomes, chunkX << 4, chunkZ << 4, 16, 16, true);
    arrayWithLength<unsigned char> biomeIds = chunk->getBiomes();
    for (unsigned int i = 0; i < biomeIds.length && i < mBiomes.length; ++i)
    {
        biomeIds.data[i] = static_cast<unsigned char>(Biome::getId(mBiomes.data[i]));
    }
    delete[] mBiomes.data;
    mBiomes.data = NULL;
    mBiomes.length = 0;
    chunk->recalcHeightmap();
    return chunk;
}

void TheEndLevelSource::postProcess(int chunkX, int chunkZ)
{
    BlockPos origin(chunkX << 4, 0, chunkZ << 4);
    mPopulationRandom->setSeed(mLevel->getSeed());
    const long long xSeed = mPopulationRandom->nextLong() / 2LL * 2LL + 1LL;
    const long long zSeed = mPopulationRandom->nextLong() / 2LL * 2LL + 1LL;
    mPopulationRandom->setSeed((static_cast<long long>(chunkX) * xSeed + static_cast<long long>(chunkZ) * zSeed) ^ mLevel->getSeed());
    if (fjIslandCheck(chunkX, chunkZ))
    {
        const double height = getIslandHeightValue(chunkX, chunkZ, 1, 1);
        if (height < -20.0 && mPopulationRandom->nextInt(14) == 0)
        {
            BlockPos pos = origin.offset(mPopulationRandom->nextInt(16) + 8, mPopulationRandom->nextInt(16) + 55, mPopulationRandom->nextInt(16) + 8);
            (void)pos;
        }
    }
}

void TheEndLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
}

std::vector<Biome::MobSpawnerData*> TheEndLevelSource::getMobsAt(MobCategory* category, const BlockPos& pos)
{
    Biome* biome = mLevel->getBiome(pos);
    return *biome->getMobs(category);
}

BlockPos* TheEndLevelSource::findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos, bool findUnexplored)
{
    (void)level;
    (void)feature;
    (void)pos;
    (void)findUnexplored;
    return NULL;
}

bool TheEndLevelSource::isPosInFeature(Level* level, const std::wstring& feature, const BlockPos& pos)
{
    (void)level;
    (void)feature;
    (void)pos;
    return false;
}

void TheEndLevelSource::recreateLogicStructuresForChunk(LevelChunk*, int, int)
{
}
