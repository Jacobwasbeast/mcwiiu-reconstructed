#include "game/world/OverworldLevelSource.hpp"

#include <cmath>
#include <cstring>
#include <algorithm>

#include "game/core/CustomizableSourceSettings.hpp"
#include "game/core/PerlinNoise.hpp"
#include "game/core/PerlinSimplexNoise.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/entity/mob/NaturalMobSpawner.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/LakeFeature.hpp"
#include "game/world/generation/LargeFeature.hpp"
#include "game/world/generation/MonsterRoomFeature.hpp"
#include "game/world/generation/OceanMonumentFeature.hpp"
#include "game/world/generation/OceanRuinFeature.hpp"
#include "game/world/generation/RandomScatteredLargeFeature.hpp"
#include "game/world/generation/StructureFeature.hpp"

void* XPhysicalAlloc(unsigned long size, unsigned long upperAddress, unsigned long alignment, unsigned int protect);
void XPhysicalFree(void* address);
void* XMemCpy(void* destination, const void* source, unsigned int count);

OverworldLevelSource::OverworldLevelSource(Level* level, long long seed, bool generateStructures, SuperflatConfig* config)
    : mLevelSize(level->getLevelData()->getXZSize()),
      mRandom(seed),
      mPopulationRandom(seed),
      mDepthNoise(new PerlinNoise(&mRandom, 16)),
      mMainNoise(new PerlinNoise(&mRandom, 16)),
      mScaleNoise(new PerlinNoise(&mRandom, 8)),
      mTemperatureNoise(new PerlinSimplexNoise(&mRandom, 4)),
      mForestNoise(new PerlinNoise(&mRandom, 10)),
      mDetailNoise(new PerlinNoise(&mRandom, 16)),
      mLevel(level),
      mGenerateStructures(generateStructures),
      mSeaLevel(level->getSeaLevel()),
      mSettings(NULL),
      mDefaultBlock(NULL),
      mCaves(NULL),
      mStronghold(NULL),
      mVillage(NULL),
      mMineshaft(NULL),
      mTemple(NULL),
      mRavines(NULL),
      mCanyons(NULL),
      mMonument(NULL),
      mMansion(NULL),
      mBuriedTreasure(NULL),
      mShipwreck(NULL),
      mOceanRuin(NULL)
{
    CustomizableSourceSettings::Builder* builder = CustomizableSourceSettings::Builder::fromString(config);
    if (builder == NULL)
    {
        builder = new CustomizableSourceSettings::Builder();
    }
    mSettings = builder->build();
    mSeaLevel = mSettings->waterLakeChance;
    level->setSeaLevel(mSeaLevel);
    delete builder;
    mDefaultBlock = Block::defaultBlockState();
    if (generateStructures)
    {
        mCaves = new LargeFeature();
        mStronghold = new StrongholdFeature();
        mVillage = new VillageFeature(mLevelSize);
        mMineshaft = new MineShaftFeature();
        mTemple = new RandomScatteredLargeFeature();
        mRavines = new LargeFeature();
        mCanyons = new LargeFeature();
        mMonument = new OceanMonumentFeature();
    }
}

OverworldLevelSource::~OverworldLevelSource()
{
    delete mCaves;
    delete mStronghold;
    delete mVillage;
    delete mMineshaft;
    delete mTemple;
    delete mRavines;
    delete mCanyons;
    delete mMonument;
    delete mMansion;
    delete mBuriedTreasure;
    delete mShipwreck;
    delete mOceanRuin;
    delete mDepthNoise;
    delete mMainNoise;
    delete mScaleNoise;
    delete mTemperatureNoise;
    delete mForestNoise;
    delete mDetailNoise;
    delete mSettings;
}

double OverworldLevelSource::getHeightFalloff(int x, int z, int* heights)
{
    int radius = mLevelSize >> 1;
    int value = std::min(std::min(std::min(std::max(x + radius, 0), std::max(radius - x - 1, 0)),
                                  std::max(z + radius, 0)), std::max(radius - z - 1, 0));
    float falloff = 0.0f;
    if (value < 32)
    {
        falloff = static_cast<float>(32 - value) * 0.03125f * 128.0f;
    }
    *heights = value;
    return falloff;
}

void OverworldLevelSource::getHeights(int x, int y, int z, arrayWithLength<Biome*>& biomes, arrayWithLength<double>& heights)
{
    int index;
    int count = biomes.length;
    for (index = 0; index < count; ++index)
    {
        double depth = biomes.data[index]->getDepth();
        double scale = biomes.data[index]->getScale();
        heights.data[index] = depth + scale * 0.5;
    }
}

void OverworldLevelSource::prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer)
{
    arrayWithLength<Biome*> biomes;
    arrayWithLength<double> heights;
    mLevel->getBiomeSource()->getBiomeBlock(biomes, chunkX << 4, chunkZ << 4, 16, 16, true);
    heights.data = new double[biomes.length];
    heights.length = biomes.length;
    getHeights(chunkX, 0, chunkZ, biomes, heights);
    int x;
    int z;
    for (x = 0; x < 16; ++x)
    {
        for (z = 0; z < 16; ++z)
        {
            int height = mLevel->getSeaLevel();
            int y;
            for (y = 0; y < height; ++y)
            {
                primer->setBlockAndData((y << 8) | (z << 4) | x, 1, 0);
            }
        }
    }
    buildSurfaces(chunkX, chunkZ, primer, biomes);
    delete[] heights.data;
    delete[] biomes.data;
}

void OverworldLevelSource::buildSurfaces(int chunkX, int chunkZ, ChunkPrimer* primer, arrayWithLength<Biome*> biomes)
{
    int x;
    int z;
    for (x = 0; x < 16; ++x)
    {
        for (z = 0; z < 16; ++z)
        {
            int index = z * 16 + x;
            if (index < static_cast<int>(biomes.length) && biomes.data[index] != NULL)
            {
                biomes.data[index]->buildSurfaceAt(mLevel, mRandom, primer, x, z, 0.0);
            }
        }
    }
}

LevelChunk* OverworldLevelSource::createChunk(int chunkX, int chunkZ)
{
    unsigned short* elements = static_cast<unsigned short*>(XPhysicalAlloc(65536, 0xFFFFFFFF, 0x1000, 4));
    std::memset(elements, 0, 65536 * sizeof(unsigned short));
    arrayWithLength<unsigned short> elementData;
    elementData.data = elements;
    elementData.length = 65536;
    ChunkPrimer primer(false, elementData);
    prepareHeights(chunkX, chunkZ, &primer);
    LevelChunk* chunk = new LevelChunk(mLevel, &primer, chunkX, chunkZ);
    XPhysicalFree(elements);
    chunk->recalcHeightmap();
    return chunk;
}

void OverworldLevelSource::lightChunk(LevelChunk* chunk)
{
    chunk->recalcHeightmap();
}

bool OverworldLevelSource::isWater(const BlockPos& pos)
{
    const BlockState* state = mLevel->getBlockState(pos);
    return state != NULL && state->getBlock() == Blocks::WATER;
}

int OverworldLevelSource::calcWaterDepths(int x, int z)
{
    int depth = 0;
    int y;
    for (y = mLevel->getSeaLevel() - 1; y >= 0 && isWater(BlockPos(x, y, z)); --y)
    {
        ++depth;
    }
    return depth;
}

void OverworldLevelSource::postProcess(int chunkX, int chunkZ)
{
    BlockPos origin(chunkX * 16, 0, chunkZ * 16);
    if (mCaves != NULL)
    {
        mCaves->apply(mLevel, chunkX, chunkZ, NULL);
    }
    if (mGenerateStructures && mMineshaft != NULL)
    {
        mMineshaft->apply(mLevel, chunkX, chunkZ, NULL);
    }
    if (mSettings != NULL && mSettings->useDungeons)
    {
        int index;
        for (index = 0; index < 8; ++index)
        {
            MonsterRoomFeature room;
            room.place(mLevel, mPopulationRandom, origin.offset(mPopulationRandom.nextInt(16) + 8,
                                                                  mPopulationRandom.nextInt(128),
                                                                  mPopulationRandom.nextInt(16) + 8));
        }
    }
}

void OverworldLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
}

std::vector<Biome::MobSpawnerData*> OverworldLevelSource::getMobsAt(MobCategory* category, const BlockPos& pos)
{
    if (mGenerateStructures && mTemple != NULL && (category == MobCategory::MONSTER || category == MobCategory::CREATURE))
    {
        std::vector<Biome::MobSpawnerData*>* mobs = mLevel->getBiome(pos)->getMobs(category);
        return mobs == NULL ? std::vector<Biome::MobSpawnerData*>() : *mobs;
    }
    std::vector<Biome::MobSpawnerData*>* mobs = mLevel->getBiome(pos)->getMobs(category);
    return mobs == NULL ? std::vector<Biome::MobSpawnerData*>() : *mobs;
}

bool OverworldLevelSource::isPosInFeature(Level* level, const std::wstring& feature, const BlockPos& pos)
{
    if (feature == L"Monument" && mMonument != NULL)
    {
        return mMonument->isInsideBoundingFeature(level, pos);
    }
    if (feature == L"Temple" && mTemple != NULL)
    {
        return mTemple->isInsideBoundingFeature(level, pos);
    }
    return false;
}

BlockPos* OverworldLevelSource::findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos, bool takeBlock)
{
    StructureFeature* structure = NULL;
    if (feature == L"Stronghold") structure = mStronghold;
    else if (feature == L"Village") structure = mVillage;
    else if (feature == L"Mineshaft") structure = mMineshaft;
    else if (feature == L"Temple") structure = mTemple;
    else if (feature == L"Monument") structure = mMonument;
    if (structure == NULL)
    {
        return NULL;
    }
    return StructureFeature::findNearestFeaturePositionBySpacing(level, structure, pos, 100, takeBlock);
}

void OverworldLevelSource::recreateLogicStructuresForChunk(LevelChunk* chunk, int chunkX, int chunkZ)
{
    if (mStronghold != NULL) mStronghold->addFeature(mLevel, chunkX, chunkZ, chunkX, chunkZ, NULL);
    if (mVillage != NULL) mVillage->addFeature(mLevel, chunkX, chunkZ, chunkX, chunkZ, NULL);
    if (chunk != NULL) chunk->recalcHeightmap();
}
