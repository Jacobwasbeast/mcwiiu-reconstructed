#include "game/world/OverworldLevelSource.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/CustomizableSourceSettings.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PerlinNoise.hpp"
#include "game/core/PerlinSimplexNoise.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/entity/mob/NaturalMobSpawner.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FallingBlock.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/BuriedTreasureFeature.hpp"
#include "game/world/generation/CanyonFeature.hpp"
#include "game/world/generation/LakeFeature.hpp"
#include "game/world/generation/LargeCaveFeature.hpp"
#include "game/world/generation/MineShaftFeature.hpp"
#include "game/world/generation/MonsterRoomFeature.hpp"
#include "game/world/generation/OceanMonumentFeature.hpp"
#include "game/world/generation/OceanRuinFeature.hpp"
#include "game/world/generation/RandomScatteredLargeFeature.hpp"
#include "game/world/generation/ShipwreckFeature.hpp"
#include "game/world/generation/StrongholdFeature.hpp"
#include "game/world/generation/UnderwaterCanyonFeature.hpp"
#include "game/world/generation/UnderwaterLargeCaveFeature.hpp"
#include "game/world/generation/VillageFeature.hpp"
#include "game/world/generation/WoodlandMansionFeature.hpp"

void* XPhysicalAlloc(unsigned long size, unsigned long upperAddress, unsigned long alignment, unsigned int protect);
void XPhysicalFree(void* address);
void XMemSet128(void* destination, int value, unsigned int count);
void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

OverworldLevelSource::OverworldLevelSource(Level* level, long long seed, bool generateStructures, SuperflatConfig* config)
    : mRandom(),
      mPopulationRandom(),
      mMinLimitNoise(NULL),
      mMaxLimitNoise(NULL),
      mMainNoise(NULL),
      mSurfaceNoise(NULL),
      mScaleNoise(NULL),
      mDepthNoise(NULL),
      mUnusedNoiseA(NULL),
      mUnusedNoiseB(NULL),
      mForestNoise(NULL),
      mLevel(NULL),
      mGenerateStructures(false),
      mGeneratorType(NULL),
      mSettings(NULL),
      mOceanBlock(NULL),
      mCaves(NULL),
      mStronghold(NULL),
      mVillage(NULL),
      mMineshaft(NULL),
      mTemple(NULL),
      mRavines(NULL),
      mUnderwaterCaves(NULL),
      mUnderwaterRavines(NULL),
      mMonument(NULL),
      mMansion(NULL),
      mBuriedTreasure(NULL),
      mShipwreck(NULL),
      mOceanRuin(NULL)
{
    mLevelSize = level->getLevelData()->getXZSize();
    mLevel = level;
    mGenerateStructures = generateStructures;
    mGeneratorType = level->getLevelData()->getGeneratorType();
    mCaves = new LargeCaveFeature();
    mStronghold = new StrongholdFeature();
    mVillage = new VillageFeature(mLevelSize);
    mMineshaft = new MineShaftFeature();
    mTemple = new RandomScatteredLargeFeature();
    mRavines = new CanyonFeature();
    mUnderwaterCaves = new UnderwaterLargeCaveFeature();
    mUnderwaterRavines = new UnderwaterCanyonFeature();
    mMonument = new OceanMonumentFeature();
    mMansion = new WoodlandMansionFeature(this);
    mBuriedTreasure = new BuriedTreasureFeature(mLevelSize);
    mShipwreck = new ShipwreckFeature(this, mMonument);
    mOceanRuin = new OceanRuinFeature();
    mRandom = Random(seed);
    mPopulationRandom = Random(seed);
    mMinLimitNoise = new PerlinNoise(&mRandom, 16);
    mMaxLimitNoise = new PerlinNoise(&mRandom, 16);
    mMainNoise = new PerlinNoise(&mRandom, 8);
    mSurfaceNoise = new PerlinSimplexNoise(&mRandom, 4);
    mScaleNoise = new PerlinNoise(&mRandom, 10);
    mDepthNoise = new PerlinNoise(&mRandom, 16);
    mForestNoise = new PerlinNoise(&mRandom, 8);
    int index = 0;
    for (int z = -2; z <= 2; ++z)
    {
        for (int x = -2; x <= 2; ++x)
        {
            mBiomeWeights[index++] = 10.0f / std::sqrt(static_cast<float>(x * x + z * z) + 0.2f);
        }
    }

    mOceanBlock = Blocks::WATER->defaultBlockState();

    if (config == NULL)
    {
        CustomizableSourceSettings::Builder* builder = new CustomizableSourceSettings::Builder();
        mSettings = new CustomizableSourceSettings(builder);
        delete builder;
    }
    else
    {
        CustomizableSourceSettings::Builder* builder = CustomizableSourceSettings::Builder::fromString(config);
        mSettings = builder->build();
        mOceanBlock = (mSettings->useLavaOceans ? Blocks::lava : Blocks::WATER)->defaultBlockState();
        mLevel->setSeaLevel(mSettings->seaLevel);
        delete builder;
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
    delete mUnderwaterCaves;
    delete mUnderwaterRavines;
    delete mMonument;
    delete mMansion;
    delete mBuriedTreasure;
    delete mShipwreck;
    delete mOceanRuin;
    delete mMinLimitNoise;
    delete mMaxLimitNoise;
    delete mMainNoise;
    delete mSurfaceNoise;
    delete mScaleNoise;
    delete mDepthNoise;
    delete mUnusedNoiseA;
    delete mUnusedNoiseB;
    delete mForestNoise;
    delete mSettings;
    delete[] mSurfaceDepths.data;
}

double OverworldLevelSource::getHeightFalloff(int x, int z, int* distance)
{
    int radius = (mLevelSize << 4) >> 1;
    int edgeDistance = std::min(std::min(std::max(x + radius, 0), std::max(radius - x - 1, 0)),
                                std::min(std::max(z + radius, 0), std::max(radius - z - 1, 0)));
    float falloff = 0.0f;
    if (edgeDistance < 32)
    {
        falloff = static_cast<float>(32 - edgeDistance) * 0.03125f * 128.0f;
    }
    *distance = edgeDistance;
    return falloff;
}

void OverworldLevelSource::getHeights(int x, int y, int z, arrayWithLength<Biome*>& biomes,
                                      arrayWithLength<double>& heights)
{
    arrayWithLength<double> depthNoise(25, true);
    arrayWithLength<double> mainNoise(425, true);
    arrayWithLength<double> minLimitNoise(425, true);
    arrayWithLength<double> maxLimitNoise(425, true);

    mDepthNoise->getRegion(depthNoise, x, z, 5, 5, mSettings->depthNoiseScaleX,
                           mSettings->depthNoiseScaleZ, mSettings->depthNoiseScaleExponent);
    mMainNoise->getRegion(mainNoise, x, y, z, 5, 17, 5,
                          mSettings->coordinateScale / mSettings->mainNoiseScaleX,
                          mSettings->heightScale / mSettings->mainNoiseScaleY,
                          mSettings->coordinateScale / mSettings->mainNoiseScaleZ);
    mMinLimitNoise->getRegion(minLimitNoise, x, y, z, 5, 17, 5,
                              mSettings->coordinateScale, mSettings->heightScale,
                              mSettings->coordinateScale);
    mMaxLimitNoise->getRegion(maxLimitNoise, x, y, z, 5, 17, 5,
                              mSettings->coordinateScale, mSettings->heightScale,
                              mSettings->coordinateScale);

    int noiseIndex = 0;
    int depthIndex = 0;
    for (int sampleX = 0; sampleX < 5; ++sampleX)
    {
        for (int sampleZ = 0; sampleZ < 5; ++sampleZ)
        {
            float biomeScale = 0.0f;
            float biomeDepth = 0.0f;
            float weightSum = 0.0f;
            Biome* center = biomes.data[sampleX + 2 + (sampleZ + 2) * 10];

            for (int offsetX = -2; offsetX <= 2; ++offsetX)
            {
                for (int offsetZ = -2; offsetZ <= 2; ++offsetZ)
                {
                    Biome* biome = biomes.data[sampleX + offsetX + 2 + (sampleZ + offsetZ + 2) * 10];
                    float depth = biome->getDepth() * mSettings->biomeDepthWeight
                                  + mSettings->biomeDepthOffset;
                    float scale = biome->getScale() * mSettings->biomeScaleWeight
                                  + mSettings->biomeScaleOffset;
                    if (mGeneratorType == LevelType::AMPLIFIED && depth > 0.0f)
                    {
                        depth = depth * 2.0f + 1.0f;
                        scale = scale * 4.0f + 1.0f;
                    }
                    float weight = mBiomeWeights[offsetX + 2 + (offsetZ + 2) * 5] / (depth + 2.0f);
                    if (biome->getDepth() > center->getDepth())
                    {
                        weight *= 0.5f;
                    }
                    biomeScale += scale * weight;
                    biomeDepth += depth * weight;
                    weightSum += weight;
                }
            }

            biomeScale /= weightSum;
            biomeDepth /= weightSum;
            biomeScale = biomeScale * 0.9f + 0.1f;
            biomeDepth = (biomeDepth * 4.0f - 1.0f) * 0.125f;

            double depthValue = depthNoise.data[depthIndex++] / 8000.0;
            if (depthValue < 0.0)
            {
                depthValue = -depthValue * 0.3;
            }
            depthValue = depthValue * 3.0 - 2.0;
            if (depthValue < 0.0)
            {
                depthValue /= 2.0;
                depthValue = std::max(depthValue, -1.0);
                depthValue /= 1.4;
                depthValue /= 2.0;
            }
            else
            {
                depthValue = std::min(depthValue, 1.0) / 8.0;
            }

            double centerHeight = (depthValue * 0.2 + biomeDepth) * mSettings->baseSize
                                  * 0.125 * 4.0 + mSettings->baseSize;

            for (int sampleY = 0; sampleY < 17; ++sampleY)
            {
                double vertical = (sampleY - centerHeight) * mSettings->stretchY * 128.0 / 256.0 / biomeScale;
                if (vertical < 0.0)
                {
                    vertical *= 4.0;
                }
                double low = minLimitNoise.data[noiseIndex] / mSettings->upperLimitScale;
                double high = maxLimitNoise.data[noiseIndex] / mSettings->lowerLimitScale;
                double blend = (mainNoise.data[noiseIndex] / mSettings->coordinateScale + 1.0) / 2.0;
                double density = Mth::clampedLerp(low, high, blend) - vertical;
                if (sampleY > 13)
                {
                    double topBlend = (sampleY - 13) / 3.0;
                    density = density * (1.0 - topBlend) - 10.0 * topBlend;
                }
                heights.data[noiseIndex++] = density;
            }
        }
    }

    delete[] depthNoise.data;
    delete[] mainNoise.data;
    delete[] minLimitNoise.data;
    delete[] maxLimitNoise.data;
}

void OverworldLevelSource::prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer)
{
    arrayWithLength<Biome*> biomes;
    PIXBeginNamedEvent(0.0f, "Getting raw biome block");
    mLevel->getBiomeSource()->getRawBiomeBlock(biomes, chunkX * 4 - 2, chunkZ * 4 - 2, 10, 10);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Get heights");
    arrayWithLength<double> heights(425, true);
    getHeights(chunkX * 4, 0, chunkZ * 4, biomes, heights);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Generating");

    for (int cellX = 0; cellX < 4; ++cellX)
    {
        for (int cellZ = 0; cellZ < 4; ++cellZ)
        {
            for (int cellY = 0; cellY < 16; ++cellY)
            {
                int base = (cellX * 5 + cellZ) * 17 + cellY;
                double a = heights.data[base];
                double b = heights.data[base + 17];
                double c = heights.data[base + 5 * 17];
                double d = heights.data[base + 6 * 17];
                double da = (heights.data[base + 1] - a) * 0.125;
                double db = (heights.data[base + 18] - b) * 0.125;
                double dc = (heights.data[base + 5 * 17 + 1] - c) * 0.125;
                double dd = (heights.data[base + 6 * 17 + 1] - d) * 0.125;

                for (int subY = 0; subY < 8; ++subY)
                {
                    double left = a;
                    double right = b;
                    double leftStep = (c - a) * 0.25;
                    double rightStep = (d - b) * 0.25;
                    int blockY = cellY * 8 + subY;

                    for (int subX = 0; subX < 4; ++subX)
                    {
                        double density = left;
                        double densityStep = (right - left) * 0.25;
                        int blockX = cellX * 4 + subX;
                        for (int subZ = 0; subZ < 4; ++subZ)
                        {
                            int blockZ = cellZ * 4 + subZ;
                            int edgeDistance;
                            double falloff = getHeightFalloff(chunkX * 16 + blockX,
                                                              chunkZ * 16 + blockZ, &edgeDistance);
                            int blockId = 0;
                            if (density > falloff)
                            {
                                blockId = 1;
                            }
                            else if (blockY < mLevel->getSeaLevel())
                            {
                                blockId = 9;
                            }
                            if (edgeDistance == 0)
                            {
                                if (blockY <= mLevel->getSeaLevel() - 10)
                                {
                                    blockId = 1;
                                }
                                else if (blockY < mLevel->getSeaLevel())
                                {
                                    blockId = 9;
                                }
                            }
                            primer->setBlockAndData((blockX << 11) | (blockZ << 7) | blockY, blockId, 0);
                            density += densityStep;
                        }
                        left += leftStep;
                        right += rightStep;
                    }
                    a += da;
                    b += db;
                    c += dc;
                    d += dd;
                }
            }
        }
    }

    PIXEndNamedEvent();
    delete[] heights.data;
    delete[] biomes.data;
}

void OverworldLevelSource::buildSurfaces(int chunkX, int chunkZ, ChunkPrimer* primer,
                                         arrayWithLength<Biome*> biomes)
{
    arrayWithLength<double> surfaceDepths(256, true);
    PIXBeginNamedEvent(0.0f, "Getting noise region");
    mSurfaceNoise->getRegion(surfaceDepths, chunkX * 16, chunkZ * 16, 16, 16,
                             0.0625, 0.0625, 0.5);
    PIXEndNamedEvent();
    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            int index = x * 16 + z;
            PIXBeginNamedEvent(0.0f, "Building surface for biome %ls",
                               biomes.data[index]->getName(false).c_str());
            biomes.data[index]->buildSurfaceAt(mLevel, mRandom, primer, chunkX * 16 + x,
                                                chunkZ * 16 + z, surfaceDepths.data[index]);
            PIXEndNamedEvent();
        }
    }
    delete[] surfaceDepths.data;
}

LevelChunk* OverworldLevelSource::createChunk(int chunkX, int chunkZ)
{
    mRandom.setSeed(static_cast<long long>(chunkX) * 341873128712LL
                    + static_cast<long long>(chunkZ) * 132897987541LL);
    PIXBeginNamedEvent(0.0f, "Setting up primer");
    unsigned short* elements = static_cast<unsigned short*>(XPhysicalAlloc(0x10000, 0xFFFFFFFF, 0x1000, 4));
    XMemSet128(elements, 0, 0x10000);
    arrayWithLength<unsigned short> elementData;
    elementData.data = elements;
    elementData.length = 0x8000;
    ChunkPrimer primer(false, elementData);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Preparing heights");
    prepareHeights(chunkX, chunkZ, &primer);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Getting biome block");
    arrayWithLength<Biome*> biomes;
    mLevel->getBiomeSource()->getBiomeBlock(biomes, chunkX << 4, chunkZ << 4, 16, 16, true);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Building surfaces");
    buildSurfaces(chunkX, chunkZ, &primer, biomes);
    PIXEndNamedEvent();
    delete[] biomes.data;

    if (mSettings->useCaves)
    {
        PIXBeginNamedEvent(0.0f, "Applying underwater caves");
        mUnderwaterCaves->apply(mLevel, chunkX, chunkZ, &primer);
        PIXEndNamedEvent();
    }
    if (mSettings->useRavines)
    {
        PIXBeginNamedEvent(0.0f, "Applying underwater ravines");
        mUnderwaterRavines->apply(mLevel, chunkX, chunkZ, &primer);
        PIXEndNamedEvent();
    }
    if (mSettings->useCaves)
    {
        PIXBeginNamedEvent(0.0f, "Applying caves");
        mCaves->apply(mLevel, chunkX, chunkZ, &primer);
        PIXEndNamedEvent();
    }
    if (mSettings->useRavines)
    {
        PIXBeginNamedEvent(0.0f, "Applying ravines");
        mRavines->apply(mLevel, chunkX, chunkZ, &primer);
        PIXEndNamedEvent();
    }
    if (mGenerateStructures)
    {
        if (mSettings->useMineShafts)
        {
            PIXBeginNamedEvent(0.0f, "Applying mineshafts");
            mMineshaft->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useVillages)
        {
            PIXBeginNamedEvent(0.0f, "Applying villages");
            mVillage->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useStrongholds)
        {
            PIXBeginNamedEvent(0.0f, "Applying strongholds");
            mStronghold->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useTemples)
        {
            PIXBeginNamedEvent(0.0f, "Applying scatterd features");
            mTemple->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useOceanRuins)
        {
            PIXBeginNamedEvent(0.0f, "Applying ruin feature");
            mOceanRuin->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useMonuments)
        {
            PIXBeginNamedEvent(0.0f, "Applying ocean monuments");
            mMonument->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useMansions)
        {
            PIXBeginNamedEvent(0.0f, "Applying woodland mansions");
            mMansion->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useShipwrecks)
        {
            PIXBeginNamedEvent(0.0f, "Applying shipwreck feature");
            mShipwreck->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
        if (mSettings->useBuriedTreasure)
        {
            PIXBeginNamedEvent(0.0f, "Applying buried treasure");
            mBuriedTreasure->apply(mLevel, chunkX, chunkZ, &primer);
            PIXEndNamedEvent();
        }
    }

    PIXBeginNamedEvent(0.0f, "Creating chunk");
    LevelChunk* chunk = new LevelChunk(mLevel, &primer, chunkX, chunkZ);
    PIXEndNamedEvent();
    XPhysicalFree(elements);
    return chunk;
}

void OverworldLevelSource::lightChunk(LevelChunk* chunk)
{
    chunk->recalcHeightmap();
}

bool OverworldLevelSource::isWater(const BlockPos& pos)
{
    const BlockState* state = mLevel->getBlockState(pos);
    return state->getBlock() == mOceanBlock->getBlock()
        && mLevel->getBlockData(pos.x, pos.y, pos.z) < 7;
}

void OverworldLevelSource::calcWaterDepths(int chunkX, int chunkZ)
{
    int seaLevel = mLevel->getSeaLevel();
    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            BlockPos center(chunkX * 16 + x + 7, seaLevel, chunkZ * 16 + z + 7);
            if (mLevel->getHeightmapPos(center).y >= 1)
            {
                continue;
            }
            bool waterEdge = false;
            BlockPos adjacent[4] = { center.west(), center.east(), center.north(), center.south() };
            for (int side = 0; side < 4; ++side)
            {
                if (mLevel->getHeightmapPos(adjacent[side]).y > 0 && isWater(adjacent[side]))
                {
                    waterEdge = true;
                    break;
                }
            }
            if (!waterEdge)
            {
                continue;
            }
            for (int dx = -5; dx <= 5; ++dx)
            {
                for (int dz = -5; dz <= 5; ++dz)
                {
                    int distance = std::abs(dx) + std::abs(dz);
                    if (distance < 6)
                    {
                        BlockPos pos = center.offset(dx, 0, dz);
                        int depth = std::min(6 - distance, 7);
                        if (isWater(pos) && mLevel->getBlockData(pos.x, pos.y, pos.z) < depth)
                        {
                            mLevel->setData(pos, depth, 3, false);
                        }
                    }
                }
            }
            mLevel->setBlockAndData(center.x, center.y, center.z, Block::getId(mOceanBlock->getBlock()), 7, 2);
            for (BlockPos pos = center.below(); pos.y >= 0; pos = pos.below())
            {
                mLevel->setBlockAndData(pos.x, pos.y, pos.z, Block::getId(mOceanBlock->getBlock()), 8, 2);
            }
        }
    }
}

void OverworldLevelSource::postProcess(int chunkX, int chunkZ)
{
    FallingBlock::fallInstantly = true;
    BlockPos origin(chunkX * 16, 0, chunkZ * 16);
    Biome* biome = mLevel->getBiome(origin.offset(16, 0, 16));
    mPopulationRandom.setSeed(mLevel->getSeed());
    long long oddX = mPopulationRandom.nextLong() / 2LL * 2LL + 1LL;
    long long oddZ = mPopulationRandom.nextLong() / 2LL * 2LL + 1LL;
    mPopulationRandom.setSeed((static_cast<long long>(chunkX) * oddX
                               + static_cast<long long>(chunkZ) * oddZ) ^ mLevel->getSeed());
    ChunkPos chunkPos(chunkX, chunkZ);
    bool villageGenerated = false;

    PIXBeginNamedEvent(0.0f, "Structure postprocessing");
    if (mGenerateStructures)
    {
        if (mSettings->useMineShafts) mMineshaft->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useVillages) villageGenerated = mVillage->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useStrongholds) mStronghold->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useTemples) mTemple->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useMonuments) mMonument->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useOceanRuins) mOceanRuin->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useMansions) mMansion->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useBuriedTreasure) mBuriedTreasure->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (mSettings->useShipwrecks) mShipwreck->postProcess(mLevel, mPopulationRandom, &chunkPos);
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Lakes");
    if (biome != Biome::DESERT && biome != Biome::DESERT_HILLS && mSettings->useWaterLakes
        && !villageGenerated && mPopulationRandom.nextInt(mSettings->waterLakeChance) == 0)
    {
        LakeFeature(Blocks::WATER).place(mLevel, mPopulationRandom,
            origin.offset(mPopulationRandom.nextInt(16) + 8, mPopulationRandom.nextInt(128),
                          mPopulationRandom.nextInt(16) + 8));
    }
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Lava");
    if (!villageGenerated && mPopulationRandom.nextInt(mSettings->lavaLakeChance / 10) == 0
        && mSettings->useLavaLakes)
    {
        BlockPos pos = origin.offset(mPopulationRandom.nextInt(16) + 8,
                                     mPopulationRandom.nextInt(mPopulationRandom.nextInt(120) + 8),
                                     mPopulationRandom.nextInt(16) + 8);
        if (pos.y < mLevel->getSeaLevel() || mPopulationRandom.nextInt(mSettings->lavaLakeChance / 8) == 0)
        {
            LakeFeature(Blocks::lava).place(mLevel, mPopulationRandom, pos);
        }
    }
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Monster rooms");
    if (mSettings->useDungeons)
    {
        for (int i = 0; i < mSettings->dungeonChance; ++i)
        {
            MonsterRoomFeature().place(mLevel, mPopulationRandom,
                origin.offset(mPopulationRandom.nextInt(16) + 8, mPopulationRandom.nextInt(128),
                              mPopulationRandom.nextInt(16) + 8));
        }
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Biome decorate");
    biome->decorate(mLevel, mPopulationRandom, origin);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Post process mobs");
    NaturalMobSpawner::spawnMobsForChunkGeneration(mLevel, biome, origin.x + 8, origin.z + 8,
                                                    16, 16, &mPopulationRandom);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Update ice and snow");
    origin = origin.offset(8, 0, 8);
    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            BlockPos top = mLevel->getTopRainBlockPos(origin.offset(x, 0, z));
            BlockPos below = top.below();
            if (mLevel->shouldFreezeIgnoreNeighbors(below))
            {
                mLevel->setBlock(below, Blocks::ice->defaultBlockState(), 2, false);
            }
            mLevel->tryToPlaceTopSnow(top, true, true);
        }
    }
    PIXEndNamedEvent();
    FallingBlock::fallInstantly = false;
}

bool OverworldLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
    return false;
}

std::vector<Biome::MobSpawnerData*> OverworldLevelSource::getMobsAt(MobCategory* category,
                                                                    const BlockPos& pos)
{
    Biome* biome = mLevel->getBiome(pos);
    if (mGenerateStructures)
    {
        if ((category == MobCategory::MONSTER || category == MobCategory::CREATURE)
            && mTemple->isSwamphut(pos))
        {
            return *mTemple->getSwamphutEnemies();
        }
        if (category == MobCategory::GUARDIAN && mSettings->useMonuments
            && mMonument->isInsideBoundingFeature(mLevel, pos))
        {
            return *mMonument->getEnemies();
        }
    }
    return *biome->getMobs(category);
}

bool OverworldLevelSource::isPosInFeature(Level*, const std::wstring& feature, const BlockPos& pos)
{
    if (!mGenerateStructures) return false;
    if (feature == L"Stronghold" && mStronghold != NULL) return mStronghold->isInsideFeature(pos);
    if (feature == L"Shipwreck" && mShipwreck != NULL) return mShipwreck->isInsideFeature(pos);
    if (feature == L"Mansion" && mMansion != NULL) return mMansion->isInsideFeature(pos);
    if (feature == L"Monument" && mMonument != NULL) return mMonument->isInsideFeature(pos);
    if (feature == L"Village" && mVillage != NULL) return mVillage->isInsideFeature(pos);
    if (feature == L"Mineshaft" && mMineshaft != NULL) return mMineshaft->isInsideFeature(pos);
    if (feature == L"Temple" && mTemple != NULL) return mTemple->isInsideFeature(pos);
    if (feature == L"Ocean_Ruin" && mOceanRuin != NULL) return mOceanRuin->isInsideFeature(pos);
    return false;
}

BlockPos* OverworldLevelSource::findNearestMapFeature(Level* level, const std::wstring& feature,
                                                       const BlockPos& pos, bool takeBlock)
{
    if (!mGenerateStructures) return NULL;
    if (feature == L"Stronghold" && mStronghold != NULL) return mStronghold->getNearestGeneratedFeature(level, pos, takeBlock);
    if (feature == L"Shipwreck" && mShipwreck != NULL) return mShipwreck->getNearestGeneratedFeature(level, pos, takeBlock);
    if (feature == L"Mansion" && mMansion != NULL) return mMansion->getNearestGeneratedFeature(level, pos, takeBlock);
    if (feature == L"Monument" && mMonument != NULL) return mMonument->getNearestGeneratedFeature(level, pos, takeBlock);
    if (feature == L"Village" && mVillage != NULL) return mVillage->getNearestGeneratedFeature(level, pos, takeBlock);
    if (feature == L"Mineshaft" && mMineshaft != NULL) return mMineshaft->getNearestGeneratedFeature(level, pos, takeBlock);
    if (feature == L"Temple" && mTemple != NULL) return mTemple->getNearestGeneratedFeature(level, pos, takeBlock);
    if (feature == L"Buried_Treasure" && mBuriedTreasure != NULL) return mBuriedTreasure->getNearestGeneratedFeature(level, pos, takeBlock);
    if (feature == L"Ocean_Ruin" && mOceanRuin != NULL) return mOceanRuin->getNearestGeneratedFeature(level, pos, takeBlock);
    return NULL;
}

void OverworldLevelSource::recreateLogicStructuresForChunk(LevelChunk*, int chunkX, int chunkZ)
{
    if (!mGenerateStructures) return;
    if (mSettings->useMineShafts) mMineshaft->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useVillages) mVillage->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useStrongholds) mStronghold->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useTemples) mTemple->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useMonuments) mMonument->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useMansions) mMansion->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useShipwrecks) mShipwreck->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useBuriedTreasure) mBuriedTreasure->apply(mLevel, chunkX, chunkZ, NULL);
    if (mSettings->useOceanRuins) mOceanRuin->apply(mLevel, chunkX, chunkZ, NULL);
}
