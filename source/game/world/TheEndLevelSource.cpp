#include "game/world/TheEndLevelSource.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/Mth.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChorusFlowerBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/EndCityFeature.hpp"
#include "game/world/generation/EndIslandFeature.hpp"
#include "game/world/TheEndDimension.hpp"

TheEndLevelSource::TheEndLevelSource(Level* level, bool generateStructures, long long seed,
                                     const BlockPos& origin)
{
    mLevelHeight = 128;
    mEndCityFeature = new EndCityFeature(this);
    mRandom = new Random(seed);
    mPopulationRandom = new Random(seed);
    mNoise1 = new PerlinNoise(mRandom, 16);
    mNoise2 = new PerlinNoise(mRandom, 16);
    mNoise3 = new PerlinNoise(mRandom, 8);
    mNoise4 = new PerlinNoise(mRandom, 10);
    mNoise5 = new PerlinNoise(mRandom, 16);
    mLevel = level;
    mGenerateStructures = generateStructures;
    mOrigin = origin;
    mIslandNoise = new SimplexNoise(mRandom);
    mEndIslandFeature = new EndIslandFeature();
}

TheEndLevelSource::~TheEndLevelSource()
{
    delete mEndCityFeature;
    delete mRandom;
    delete mPopulationRandom;
    delete mNoise1;
    delete mNoise2;
    delete mNoise3;
    delete mNoise4;
    delete mNoise5;
    delete mIslandNoise;
    delete mEndIslandFeature;
    delete[] mBiomes.data;
}

double TheEndLevelSource::fjIslandProximity(int x, int z)
{
    ChunkPos* area = TheEndDimension::closestAreaToPoint(x, z);
    if (area->x == 0 && area->z == 0)
    {
        return 12.0;
    }

    const double xDistance = std::fabs(static_cast<double>(x) + 0.5 - area->x);
    const double zDistance = std::fabs(static_cast<double>(z) + 0.5 - area->z);
    return std::max(xDistance, zDistance);
}

bool TheEndLevelSource::fjIslandCheck(int x, int z)
{
    return fjIslandProximity(x, z) < 3.0;
}

double TheEndLevelSource::getIslandHeightValue(int x, int z, int xOffset, int zOffset)
{
    ChunkPos* area = TheEndDimension::closestAreaToPoint(x, z);
    const float sampleX = static_cast<float>(x * 2 + xOffset);
    const float sampleZ = static_cast<float>(z * 2 + zOffset);

    if (area->x == 0 && area->z == 0)
    {
        return -static_cast<double>(std::sqrt(sampleX * sampleX + sampleZ * sampleZ) * 9.0f - 100.0f);
    }

    double height = Mth::clamp(
        -static_cast<double>(std::sqrt(sampleX * sampleX + sampleZ * sampleZ) * 8.0f - 100.0f),
        -100.0, 80.0);

    for (int islandX = x - 4; islandX <= x + 4; ++islandX)
    {
        for (int islandZ = z - 4; islandZ <= z + 4; ++islandZ)
        {
            if (fjIslandCheck(islandX, islandZ))
            {
                const float scale = static_cast<float>(
                    (static_cast<int>(std::fabs(static_cast<double>(islandX)) * 3439.0 +
                                      std::fabs(static_cast<double>(islandZ)) * 147.0) % 6) + 18);
                const float distanceX = static_cast<float>(xOffset + (x - islandX) * 2);
                const float distanceZ = static_cast<float>(zOffset + (z - islandZ) * 2);
                const double islandHeight = Mth::clamp(
                    -static_cast<double>(std::sqrt(distanceX * distanceX + distanceZ * distanceZ) *
                                         scale - 100.0f),
                    -100.0, 80.0);
                height = std::max(height, islandHeight);
            }
        }
    }

    return height;
}

arrayWithLength<double> TheEndLevelSource::getHeights(arrayWithLength<double> values, int x, int y,
                                                       int z, int xSize, int ySize, int zSize)
{
    const unsigned int count = static_cast<unsigned int>(xSize * ySize * zSize);
    if (values.data == NULL)
    {
        values.data = new double[count]();
        values.length = count;
    }

    arrayWithLength<double> unusedDepth = mNoise4->getRegion(
        arrayWithLength<double>(), x, z, xSize, zSize, 1.121, 1.121, 0.5);
    arrayWithLength<double> unusedScale = mNoise5->getRegion(
        arrayWithLength<double>(), x, z, xSize, zSize, 200.0, 200.0, 0.5);
    arrayWithLength<double> blend = mNoise3->getRegion(
        arrayWithLength<double>(), x, y, z, xSize, ySize, zSize,
        17.110300000000002, 4.277575000000001, 17.110300000000002);
    arrayWithLength<double> lower = mNoise1->getRegion(
        arrayWithLength<double>(), x, y, z, xSize, ySize, zSize, 684.412, 1368.824, 684.412);
    arrayWithLength<double> upper = mNoise2->getRegion(
        arrayWithLength<double>(), x, y, z, xSize, ySize, zSize, 684.412, 1368.824, 684.412);

    unsigned int index = 0;
    const int middle = ySize / 2;
    for (int localX = 0; localX < xSize; ++localX)
    {
        for (int localZ = 0; localZ < zSize; ++localZ)
        {
            double islandHeight = getIslandHeightValue(x / 2, z / 2, localX, localZ);
            islandHeight = Mth::clamp(islandHeight, -100.0, 80.0);

            for (int localY = 0; localY < ySize; ++localY)
            {
                const double selector = (blend.data[index] / 10.0 + 1.0) * 0.5;
                const double low = lower.data[index] / 512.0;
                const double high = upper.data[index] / 512.0;
                double density;
                if (selector < 0.0)
                {
                    density = low;
                }
                else if (selector > 1.0)
                {
                    density = high;
                }
                else
                {
                    density = low + (high - low) * selector;
                }

                density = density - 8.0 + islandHeight;
                if (localY > middle - 2)
                {
                    const double fade = Mth::clamp(
                        static_cast<double>(localY + 2 - middle) * 0.015625, 0.0, 1.0);
                    density = density * (1.0 - fade) - 3000.0 * fade;
                }
                if (localY < 8)
                {
                    const double fade = static_cast<double>(8 - localY) / 7.0;
                    density = density * (1.0 - fade) - 30.0 * fade;
                }

                const int worldX = (x + localX) * 8;
                const int worldY = (y + localY) * 4;
                const int worldZ = (z + localZ) * 8;
                const double originFade = Mth::clamp(
                    (19.0 - BlockPos::ZERO.dist(worldX, worldY, worldZ)) * 0.0625, 0.0, 1.0);
                const double originDensity = BlockPos::ZERO.y < worldY + 1 ? -50.0 : 50.0;
                values.data[index] = density * (1.0 - originFade) + originDensity * originFade;
                ++index;
            }
        }
    }

    delete[] blend.data;
    delete[] lower.data;
    delete[] upper.data;
    delete[] unusedDepth.data;
    delete[] unusedScale.data;
    return values;
}

void TheEndLevelSource::prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer)
{
    arrayWithLength<double> heights = getHeights(
        arrayWithLength<double>(), chunkX * 2, 0, chunkZ * 2, 3, 33, 3);
    const BlockState* endStone = Blocks::end_stone->defaultBlockState();
    const BlockState* air = Blocks::AIR->defaultBlockState();

    for (int cellX = 0; cellX < 2; ++cellX)
    {
        for (int cellZ = 0; cellZ < 2; ++cellZ)
        {
            for (int cellY = 0; cellY < 32; ++cellY)
            {
                double lowerNear = heights.data[(cellX * 3 + cellZ) * 33 + cellY];
                double lowerFar = heights.data[(cellX * 3 + cellZ + 1) * 33 + cellY];
                double upperNear = heights.data[((cellX + 1) * 3 + cellZ) * 33 + cellY];
                double upperFar = heights.data[((cellX + 1) * 3 + cellZ + 1) * 33 + cellY];
                const double lowerNearStep =
                    (heights.data[(cellX * 3 + cellZ) * 33 + cellY + 1] - lowerNear) * 0.25;
                const double lowerFarStep =
                    (heights.data[(cellX * 3 + cellZ + 1) * 33 + cellY + 1] - lowerFar) * 0.25;
                const double upperNearStep =
                    (heights.data[((cellX + 1) * 3 + cellZ) * 33 + cellY + 1] - upperNear) * 0.25;
                const double upperFarStep =
                    (heights.data[((cellX + 1) * 3 + cellZ + 1) * 33 + cellY + 1] - upperFar) * 0.25;

                for (int subY = 0; subY < 4; ++subY)
                {
                    double near = lowerNear;
                    double far = lowerFar;
                    const double nearStep = (upperNear - lowerNear) * 0.125;
                    const double farStep = (upperFar - lowerFar) * 0.125;

                    for (int subX = 0; subX < 8; ++subX)
                    {
                        double density = near;
                        const double densityStep = (far - near) * 0.125;
                        for (int subZ = 0; subZ < 8; ++subZ)
                        {
                            primer->setState(cellX * 8 + subX, cellY * 4 + subY,
                                             cellZ * 8 + subZ,
                                             density > 0.0 ? endStone : air);
                            density += densityStep;
                        }
                        near += nearStep;
                        far += farStep;
                    }

                    lowerNear += lowerNearStep;
                    lowerFar += lowerFarStep;
                    upperNear += upperNearStep;
                    upperFar += upperFarStep;
                }
            }
        }
    }

    delete[] heights.data;
}

void TheEndLevelSource::buildSurfaces(ChunkPrimer* primer)
{
    const BlockState* top = Blocks::end_stone->defaultBlockState();
    const BlockState* filler = Blocks::end_stone->defaultBlockState();
    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            int depth = -1;
            for (int y = 127; y >= 0; --y)
            {
                const BlockState* state = primer->getState(x, y, z);
                Block* block = state->getBlock();
                if (block->mMaterial == Material::AIR)
                {
                    depth = -1;
                }
                else if (block == Blocks::stone)
                {
                    if (depth == -1)
                    {
                        depth = 1;
                        primer->setState(x, y, z, top);
                    }
                    else if (depth > 0)
                    {
                        --depth;
                        primer->setState(x, y, z, filler);
                    }
                }
            }
        }
    }
}

LevelChunk* TheEndLevelSource::createChunk(int chunkX, int chunkZ)
{
    mRandom->setSeed(static_cast<long long>(chunkX) * 341873128712LL +
                     static_cast<long long>(chunkZ) * 132897987541LL);
    arrayWithLength<unsigned short> elements(65536, true);
    ChunkPrimer primer(true, elements);
    mLevel->getBiomeSource()->getBiomeBlock(mBiomes, chunkX << 4, chunkZ << 4, 16, 16, true);
    prepareHeights(chunkX, chunkZ, &primer);
    buildSurfaces(&primer);
    if (mGenerateStructures)
    {
        mEndCityFeature->apply(mLevel, chunkX, chunkZ, &primer);
    }

    LevelChunk* chunk = new LevelChunk(mLevel, &primer, chunkX, chunkZ);
    arrayWithLength<unsigned char> biomeIds = chunk->getBiomes();
    for (unsigned int index = 0; index < biomeIds.length; ++index)
    {
        biomeIds.data[index] = static_cast<unsigned char>(Biome::getId(mBiomes.data[index]));
    }
    chunk->recalcHeightmap();
    return chunk;
}

void TheEndLevelSource::postProcess(int chunkX, int chunkZ)
{
    Level::setInstaTick(true);
    const BlockPos origin(chunkX << 4, 0, chunkZ << 4);
    mPopulationRandom->setSeed(mLevel->getSeed());
    const long long xSeed = mPopulationRandom->nextLong() / 2LL * 2LL + 1LL;
    const long long zSeed = mPopulationRandom->nextLong() / 2LL * 2LL + 1LL;
    mPopulationRandom->setSeed(
        (static_cast<long long>(chunkX) * xSeed + static_cast<long long>(chunkZ) * zSeed) ^
        mLevel->getSeed());

    if (mGenerateStructures)
    {
        ChunkPos chunkPos(chunkX, chunkZ);
        mEndCityFeature->postProcess(mLevel, *mPopulationRandom, &chunkPos);
    }
    mLevel->getBiome(origin.offset(16, 0, 16))->decorate(mLevel, *mPopulationRandom, origin);

    if (fjIslandCheck(chunkX, chunkZ))
    {
        if (getIslandHeightValue(chunkX, chunkZ, 1, 1) < -20.0 &&
            mPopulationRandom->nextInt(14) == 0)
        {
            mEndIslandFeature->place(
                mLevel, *mPopulationRandom,
                origin.offset(mPopulationRandom->nextInt(16) + 8,
                              mPopulationRandom->nextInt(16) + 55,
                              mPopulationRandom->nextInt(16) + 8));
            if (mPopulationRandom->nextInt(4) == 0)
            {
                mEndIslandFeature->place(
                    mLevel, *mPopulationRandom,
                    origin.offset(mPopulationRandom->nextInt(16) + 8,
                                  mPopulationRandom->nextInt(16) + 55,
                                  mPopulationRandom->nextInt(16) + 8));
            }
        }

        if (getIslandHeightValue(chunkX, chunkZ, 1, 1) > 40.0)
        {
            const int plants = mPopulationRandom->nextInt(5);
            for (int index = 0; index < plants; ++index)
            {
                const int x = mPopulationRandom->nextInt(16) + 8;
                const int z = mPopulationRandom->nextInt(16) + 8;
                const BlockPos top = mLevel->getHeightmapPos(origin.offset(x, 0, z));
                if (top.y > 0 && mLevel->isEmptyBlock(origin.offset(x, top.y, z)) &&
                    mLevel->getBlockState(origin.offset(x, top.y - 1, z))->getBlock() == Blocks::end_stone)
                {
                    ChorusFlowerBlock::generatePlant(
                        mLevel, origin.offset(x, top.y, z), mPopulationRandom, 8);
                }
            }
        }
    }
    Level::setInstaTick(false);
}

bool TheEndLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
    return false;
}

std::vector<Biome::MobSpawnerData*> TheEndLevelSource::getMobsAt(
    MobCategory* category, const BlockPos& pos)
{
    return *mLevel->getBiome(pos)->getMobs(category);
}

BlockPos* TheEndLevelSource::findNearestMapFeature(Level* level, const std::wstring& feature,
                                                    const BlockPos& pos, bool findUnexplored)
{
    if (feature == L"EndCity" && mEndCityFeature != NULL)
    {
        return mEndCityFeature->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    return NULL;
}

void TheEndLevelSource::recreateLogicStructuresForChunk(LevelChunk*, int, int)
{
}

bool TheEndLevelSource::isPosInFeature(Level*, const std::wstring& feature, const BlockPos& pos)
{
    return feature == L"EndCity" && mEndCityFeature != NULL &&
           mEndCityFeature->isInsideFeature(pos);
}
