#include "game/world/biome/OceanFrozenBiome.hpp"

#include "game/core/PerlinSimplexNoise.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"

#include <cmath>

OceanFrozenBiome::OceanFrozenBiome(EBiomeIDs id, bool deep, BiomeProperties* properties)
    : Biome(id, eBiomeType_Ocean, properties),
      mDeep(deep),
      mSeed(0),
      mRandom(NULL),
      mIcebergsNoise(NULL),
      mIcebergsNoise2(NULL),
      mIcebergNoise(new PerlinSimplexNoise(0LL, 3))
{
    clearDefaultFriendlies();
    mCreatures.push_back(new MobSpawnerData(26, 0x0D502004ULL, 3, 5));
    mCreatures.push_back(new MobSpawnerData(75, 0x0D502002ULL, 4, 7));
    mWaterCreatures.push_back(new MobSpawnerData(10, 0x0D502001ULL, 2, 4));
    mMonsters.push_back(new MobSpawnerData(10, 0x5D550044ULL, 2, 4));
    mMonsters.push_back(new MobSpawnerData(120, 0x5D540443ULL, 1, 2));
    mAmbientCreatures.push_back(new MobSpawnerData(5, 0x2D680006ULL, 1, 2));
    clearDefaultFriendlies();
    mRunDepth = 0;
    mSnowMeltRate = 1.0F;
    mMinSnowAccumulation = mDeep ? 0.5F : 0.3F;
    mMaxSnowAccumulation = mDeep ? 0.95F : 0.9F;
}

OceanFrozenBiome::~OceanFrozenBiome()
{
    delete mIcebergsNoise;
    delete mIcebergsNoise2;
    delete mRandom;
    delete mIcebergNoise;
}

float OceanFrozenBiome::getCreatureProbability()
{
    return 0.1F;
}

void OceanFrozenBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    Biome::decorate(level, random, pos);
}

void OceanFrozenBiome::buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise)
{
    long long seed = level->getSeed();
    if (mIcebergsNoise != NULL && mIcebergsNoise2 != NULL && mSeed != seed)
    {
        delete mIcebergsNoise;
        delete mIcebergsNoise2;
        delete mRandom;
        mRandom = new Random(seed);
        mIcebergsNoise = new PerlinSimplexNoise(mRandom, 4);
        mIcebergsNoise2 = new PerlinSimplexNoise(mRandom, 1);
    }
    else if (mIcebergsNoise == NULL || mIcebergsNoise2 == NULL)
    {
        delete mIcebergsNoise;
        delete mIcebergsNoise2;
        delete mRandom;
        mRandom = new Random(seed);
        mIcebergsNoise = new PerlinSimplexNoise(mRandom, 4);
        mIcebergsNoise2 = new PerlinSimplexNoise(mRandom, 1);
    }
    mSeed = seed;

    int seaLevel = level->getSeaLevel();
    BlockPos seaPos(x, seaLevel, z);
    float temperature = getTemperature(seaPos);
    double icebergNoise = mIcebergsNoise->getValue((double)x * 0.1, (double)z * 0.1);
    double icebergNoise2 = mIcebergsNoise2->getValue((double)x * 0.9765625, (double)z * 0.9765625);
    double height = icebergNoise;
    if (height < 0.0)
    {
        height = -height;
    }
    if (height < 0.7)
    {
        height = 0.7;
    }

    float icebergHeight = (float)(height * height * 0.00087890625);
    float icebergWidth = (float)(ceil((float)(icebergNoise2 < 0.0 ? -icebergNoise2 : icebergNoise2) * 40.0) + 14.0);
    if (icebergWidth + 7.0F < icebergHeight)
    {
        icebergHeight = icebergWidth + 7.0F;
    }
    if (temperature > 0.1F)
    {
        icebergHeight -= 2.0F;
    }

    int snowLayer = 126;
    int iceLayer = 127;
    int snowCount = 0;
    int snowStart = seaLevel + 18 + random.nextInt(10);
    const int column = (z & 15) << 7 | (x & 15) << 11;

    for (int y = 127; y >= 64; y -= 2)
    {
        int topIndex = column | iceLayer;
        int bottomIndex = column | snowLayer;
        if (random.nextInt(2) + 1 >= iceLayer)
        {
            primer->setBlockAndData(topIndex, 174, 0);
        }
        else
        {
            const BlockState* state = primer->getState(x & 15, iceLayer, z & 15);
            Block* block = state == NULL ? NULL : state->getBlock();
            if (block == Blocks::WATER && iceLayer < icebergHeight && random.nextFloat() > 0.01F)
            {
                primer->setBlockAndData(topIndex, 174, 0);
            }
            else if (block == Blocks::stone && iceLayer < seaLevel && icebergHeight != 0.0F && random.nextFloat() > 0.15F)
            {
                primer->setBlockAndData(topIndex, 174, 0);
            }
        }

        const BlockState* state = primer->getState(x & 15, snowLayer, z & 15);
        Block* block = state == NULL ? NULL : state->getBlock();
        if (block == Blocks::WATER && snowLayer < icebergHeight && random.nextFloat() > 0.01F)
        {
            primer->setBlockAndData(bottomIndex, 174, 0);
        }
        else if (block == Blocks::stone && snowLayer < seaLevel && icebergHeight != 0.0F && random.nextFloat() > 0.15F)
        {
            primer->setBlockAndData(bottomIndex, 174, 0);
        }

        if (primer->getState(x & 15, iceLayer, z & 15)->getBlock() == Blocks::packed_ice && snowCount <= random.nextInt(4) + 2 && snowStart < iceLayer)
        {
            primer->setBlockAndData(topIndex, 80, 0);
            ++snowCount;
        }
        if (primer->getState(x & 15, snowLayer, z & 15)->getBlock() == Blocks::packed_ice && snowCount <= random.nextInt(4) + 2 && snowStart < snowLayer)
        {
            primer->setBlockAndData(bottomIndex, 80, 0);
            ++snowCount;
        }
        iceLayer -= 2;
        snowLayer -= 2;
    }

    Biome::buildSurfaceAt(level, random, primer, x, z, noise);
}

float OceanFrozenBiome::getTemperature(const BlockPos& pos)
{
    float temperature = getDefaultBiomeTemperature();
    double noise = mIcebergNoise->getValue((double)pos.x * 0.05, (double)pos.z * 0.05);
    double globalNoise = mIcebergNoise->getValue((double)pos.x * 0.2, (double)pos.z * 0.2);
    if ((float)noise + (float)globalNoise < 0.1F)
    {
        noise = mIcebergNoise->getValue((double)pos.x * 0.09, (double)pos.z * 0.09);
        if ((float)noise < 0.8F)
        {
            temperature = 0.2F;
        }
    }
    if (pos.y > 64)
    {
        double heightNoise = mIcebergNoise->getValue((double)pos.x * 0.125, (double)pos.z * 0.125);
        return temperature - (float)((heightNoise * 4.0 + (double)pos.y - 64.0) * 0.05 / 30.0);
    }
    return temperature;
}

Biome::EBiomeBaseClass OceanFrozenBiome::getBaseClass()
{
    return eBiomeBaseClass_OceanFrozen;
}

Biome::TemperatureCategory OceanFrozenBiome::getTemperatureCategory() const
{
    return eTemperatureCategory_Ocean;
}
