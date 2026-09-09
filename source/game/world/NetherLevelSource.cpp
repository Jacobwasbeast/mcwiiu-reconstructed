#include "game/world/NetherLevelSource.hpp"

#include <cmath>

#include "game/core/CMinecraftApp.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/core/Bush.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/generation/BushFeature.hpp"
#include "game/world/generation/HellFireFeature.hpp"
#include "game/world/generation/HellPortalFeature.hpp"
#include "game/world/generation/HellSpringFeature.hpp"
#include "game/world/generation/LargeHellCaveFeature.hpp"
#include "game/world/generation/LightGemFeature.hpp"
#include "game/world/generation/OreFeature.hpp"

NetherLevelSource::NetherLevelSource(Level* level, bool generateStructures, long long seed)
    : mLevelHeight(static_cast<int>(std::ceil((static_cast<float>(level->getHeight()) - 176.0f) /
                                             (static_cast<float>(level->getMaxBuildHeight()) - 176.0f)))),
      mRandom(),
      mPopulationRandom(seed),
      mGenerateStructures(generateStructures),
      mLevel(level),
      mNoise1(new PerlinNoise(&mRandom, 16)),
      mNoise2(new PerlinNoise(&mRandom, 16)),
      mNoise3(new PerlinNoise(&mRandom, 8)),
      mNoise4(new PerlinNoise(&mRandom, 4)),
      mNoise5(new PerlinNoise(&mRandom, 4)),
      mNoise6(new PerlinNoise(&mRandom, 10)),
      mNoise7(new PerlinNoise(&mRandom, 16)),
      mFireFeature(new HellFireFeature()),
      mLightGemFeature(new LightGemFeature()),
      mHellPortalFeature(new HellPortalFeature()),
      mQuartzOre(new OreFeature(Blocks::quartz_ore->defaultBlockState(), 14)),
      mMagmaOre(new OreFeature(Blocks::magma->defaultBlockState(), 33)),
      mLavaSpring(new HellSpringFeature(Blocks::flowing_lava, true)),
      mLavaSpringOpen(new HellSpringFeature(Blocks::flowing_lava, false)),
      mBrownMushroom(new BushFeature(static_cast<Bush*>(Blocks::brown_mushroom))),
      mRedMushroom(new BushFeature(static_cast<Bush*>(Blocks::red_mushroom))),
      mNetherBridgeFeature(new NetherBridgeFeature()),
      mCaves(new LargeHellCaveFeature())
{
    mLevel->setSeaLevel(63);
}

NetherLevelSource::~NetherLevelSource()
{
    delete mNoise1;
    delete mNoise2;
    delete mNoise3;
    delete mNoise4;
    delete mNoise5;
    delete mNoise6;
    delete mNoise7;
    delete mFireFeature;
    delete mLightGemFeature;
    delete mHellPortalFeature;
    delete mQuartzOre;
    delete mMagmaOre;
    delete mLavaSpring;
    delete mLavaSpringOpen;
    delete mBrownMushroom;
    delete mRedMushroom;
    delete mNetherBridgeFeature;
    delete mCaves;
}

arrayWithLength<double> NetherLevelSource::getHeights(arrayWithLength<double> values, int x, int y, int z,
                                                       int xSize, int ySize, int zSize)
{
    const unsigned int count = static_cast<unsigned int>(xSize * ySize * zSize);
    if (values.data == NULL || values.length != count)
    {
        delete[] values.data;
        values.data = count == 0 ? NULL : new double[count]();
        values.length = count;
    }

    arrayWithLength<double> lower = mNoise2->getRegion(arrayWithLength<double>(), x, y, z,
                                                        xSize, ySize, zSize, 684.412, 2053.236, 0.5);
    arrayWithLength<double> upper = mNoise1->getRegion(arrayWithLength<double>(), x, y, z,
                                                        xSize, ySize, zSize, 684.412, 2053.236, 0.5);
    arrayWithLength<double> depth = mNoise3->getRegion(arrayWithLength<double>(), x, y, z,
                                                        xSize, ySize, zSize, 684.412, 2053.236, 0.5);
    for (int ix = 0; ix < xSize; ++ix)
    {
        for (int iz = 0; iz < zSize; ++iz)
        {
            for (int iy = 0; iy < ySize; ++iy)
            {
                const int index = (ix * zSize + iz) * ySize + iy;
                const double vertical = static_cast<double>(y + iy) / static_cast<double>(ySize);
                double value = lower.data[index] * (1.0 - vertical) + upper.data[index] * vertical;
                value -= depth.data[index] * 0.5;
                value += static_cast<double>(mLevelHeight - y) * 0.25;
                values.data[index] = value;
            }
        }
    }
    delete[] lower.data;
    delete[] upper.data;
    delete[] depth.data;
    return values;
}

void NetherLevelSource::prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer)
{
    arrayWithLength<double> heights = getHeights(arrayWithLength<double>(), chunkX * 4, 0, chunkZ * 4, 5, 17, 5);
    const int seaLevel = mLevel->getSeaLevel();
    for (int sectionX = 0; sectionX < 4; ++sectionX)
    {
        for (int sectionZ = 0; sectionZ < 4; ++sectionZ)
        {
            const int index = sectionX * 5 + sectionZ;
            for (int localX = 0; localX < 4; ++localX)
            {
                for (int localZ = 0; localZ < 4; ++localZ)
                {
                    for (int y = 0; y < 16; ++y)
                    {
                        const int height = y + 1;
                        const double xValue = heights.data[index * 17 + height];
                        const double zValue = heights.data[(index + 1) * 17 + height];
                        const double nextX = heights.data[(index + 5) * 17 + height];
                        const double nextZ = heights.data[(index + 6) * 17 + height];
                        const double blend = static_cast<double>(localX) * 0.25;
                        const double a = xValue + (nextX - xValue) * blend;
                        const double b = zValue + (nextZ - zValue) * blend;
                        const double value = a + (b - a) * static_cast<double>(localZ) * 0.25;
                        for (int dy = 0; dy < 4; ++dy)
                        {
                            const int worldY = y * 4 + dy;
                            const double adjusted = value + static_cast<double>(dy) * 0.25;
                            const BlockState* state = adjusted > 0.0 ? Blocks::netherrack->defaultBlockState() : NULL;
                            if (worldY < seaLevel && state == NULL)
                            {
                                state = Blocks::lava->defaultBlockState();
                            }
                            primer->setState(localX * 4 + sectionX * 4, worldY,
                                             localZ * 4 + sectionZ * 4, state);
                        }
                    }
                }
            }
        }
    }
    delete[] heights.data;
}

void NetherLevelSource::buildSurfaces(int chunkX, int chunkZ, ChunkPrimer* primer)
{
    arrayWithLength<double> noise = mNoise4->getRegion(arrayWithLength<double>(), chunkX * 16, chunkZ * 16,
                                                        16, 16, 0.0625, 0.0625, 1.0);
    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            int depth = -1;
            int thickness = static_cast<int>(noise.data[x * 16 + z] * 3.0 + mRandom.nextDouble() * 0.25 + 3.0);
            for (int y = 127; y >= 0; --y)
            {
                const BlockState* state = primer->getState(x, y, z);
                if (state == NULL || state->getBlock() == Blocks::AIR)
                {
                    depth = -1;
                }
                else if (state->getBlock() == Blocks::netherrack)
                {
                    if (depth == -1)
                    {
                        depth = thickness;
                        if (y >= 0 && y < 127)
                        {
                            const BlockState* top = Blocks::netherrack->defaultBlockState();
                            const BlockState* filler = Blocks::netherrack->defaultBlockState();
                            if (thickness <= 0)
                            {
                                top = Blocks::AIR->defaultBlockState();
                            }
                            else if (y < mLevel->getSeaLevel() - 3)
                            {
                                top = Blocks::soul_sand->defaultBlockState();
                                filler = Blocks::soul_sand->defaultBlockState();
                            }
                            primer->setState(x, y, z, top);
                            if (depth > 0)
                            {
                                --depth;
                                primer->setState(x, y - 1, z, filler);
                            }
                        }
                    }
                    else if (depth > 0)
                    {
                        --depth;
                        primer->setState(x, y, z, Blocks::netherrack->defaultBlockState());
                    }
                }
            }
        }
    }
    delete[] noise.data;
}

void NetherLevelSource::lightChunk(LevelChunk* chunk)
{
    chunk->recalcHeightmap();
}

LevelChunk* NetherLevelSource::createChunk(int chunkX, int chunkZ)
{
    mRandom.setSeed(static_cast<long long>(chunkX) * -1722452168LL + static_cast<long long>(chunkZ) * -245973051LL);
    arrayWithLength<unsigned short> elements(65536, true);
    ChunkPrimer primer(false, elements);
    prepareHeights(chunkX, chunkZ, &primer);
    buildSurfaces(chunkX, chunkZ, &primer);
    mCaves->apply(mLevel, chunkX, chunkZ, &primer);
    if (mGenerateStructures)
    {
        mNetherBridgeFeature->addFeature(mLevel, chunkX, chunkZ, chunkX, chunkZ, &primer);
    }
    LevelChunk* chunk = new LevelChunk(mLevel, &primer, chunkX, chunkZ);
    chunk->setCheckAllLight();
    return chunk;
}

void NetherLevelSource::postProcess(int chunkX, int chunkZ)
{
    Level::setInstaTick(true);
    const BlockPos origin(chunkX << 4, 0, chunkZ << 4);
    Biome* biome = mLevel->getBiome(origin.offset(16, 0, 16));
    ChunkPos chunkPos(chunkX, chunkZ);
    mPopulationRandom.setSeed(mLevel->getSeed());
    const long long xSeed = mPopulationRandom.nextLong() / 2LL * 2LL + 1LL;
    const long long zSeed = mPopulationRandom.nextLong() / 2LL * 2LL + 1LL;
    mPopulationRandom.setSeed((static_cast<long long>(chunkX) * xSeed +
                               static_cast<long long>(chunkZ) * zSeed) ^ mLevel->getSeed());

    mNetherBridgeFeature->postProcess(mLevel, mPopulationRandom, &chunkPos);

    for (int i = 0; i < 8; ++i)
    {
        mLavaSpringOpen->place(mLevel, mPopulationRandom,
                               origin.offset(mPopulationRandom.nextInt(16) + 8,
                                             mPopulationRandom.nextInt(120) + 4,
                                             mPopulationRandom.nextInt(16) + 8));
    }
    for (int i = 0; i < mPopulationRandom.nextInt(mPopulationRandom.nextInt(10) + 1) + 1; ++i)
    {
        mFireFeature->place(mLevel, mPopulationRandom,
                            origin.offset(mPopulationRandom.nextInt(16) + 8,
                                          mPopulationRandom.nextInt(120) + 4,
                                          mPopulationRandom.nextInt(16) + 8));
    }
    for (int i = 0; i < mPopulationRandom.nextInt(mPopulationRandom.nextInt(10) + 1); ++i)
    {
        mLightGemFeature->place(mLevel, mPopulationRandom,
                                origin.offset(mPopulationRandom.nextInt(16) + 8,
                                              mPopulationRandom.nextInt(120) + 4,
                                              mPopulationRandom.nextInt(16) + 8));
    }
    for (int i = 0; i < 10; ++i)
    {
        mHellPortalFeature->place(mLevel, mPopulationRandom,
                                  origin.offset(mPopulationRandom.nextInt(16) + 8,
                                                mPopulationRandom.nextInt(128),
                                                mPopulationRandom.nextInt(16) + 8));
    }
    if (mPopulationRandom.nextBoolean())
    {
        mBrownMushroom->place(mLevel, mPopulationRandom,
                              origin.offset(mPopulationRandom.nextInt(16) + 8,
                                            mPopulationRandom.nextInt(128),
                                            mPopulationRandom.nextInt(16) + 8));
    }
    if (mPopulationRandom.nextBoolean())
    {
        mRedMushroom->place(mLevel, mPopulationRandom,
                            origin.offset(mPopulationRandom.nextInt(16) + 8,
                                          mPopulationRandom.nextInt(128),
                                          mPopulationRandom.nextInt(16) + 8));
    }
    for (int i = 0; i < 16; ++i)
    {
        mQuartzOre->place(mLevel, mPopulationRandom,
                          origin.offset(mPopulationRandom.nextInt(16),
                                        mPopulationRandom.nextInt(108) + 10,
                                        mPopulationRandom.nextInt(16)));
    }
    const int magmaHeight = mLevel->getSeaLevel() / 2 - 4;
    for (int i = 0; i < 4; ++i)
    {
        mMagmaOre->place(mLevel, mPopulationRandom,
                         origin.offset(mPopulationRandom.nextInt(16),
                                       magmaHeight + mPopulationRandom.nextInt(10),
                                       mPopulationRandom.nextInt(16)));
    }
    for (int i = 0; i < 16; ++i)
    {
        mLavaSpring->place(mLevel, mPopulationRandom,
                           origin.offset(mPopulationRandom.nextInt(16),
                                         mPopulationRandom.nextInt(108) + 10,
                                         mPopulationRandom.nextInt(16)));
    }

    biome->decorate(mLevel, mPopulationRandom, BlockPos(chunkX << 4, 0, chunkZ << 4));
    Level::setInstaTick(false);
    g_pApp->processSchematics(mLevel->getChunk(chunkX, chunkZ));
}

bool NetherLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
    return false;
}

std::vector<Biome::MobSpawnerData*> NetherLevelSource::getMobsAt(MobCategory* category, const BlockPos& pos)
{
    if (category == MobCategory::MONSTER && mNetherBridgeFeature->isInsideFeature(pos))
    {
        return *mNetherBridgeFeature->getBridgeEnemies();
    }
    return *mLevel->getBiome(pos)->getMobs(category);
}

BlockPos* NetherLevelSource::findNearestMapFeature(Level* level, const std::wstring& feature,
                                                    const BlockPos& pos, bool findUnexplored)
{
    if (feature == L"Fortress")
    {
        return mNetherBridgeFeature->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    return NULL;
}

bool NetherLevelSource::isPosInFeature(Level*, const std::wstring& feature, const BlockPos& pos)
{
    return feature == L"Fortress" && mNetherBridgeFeature->isInsideFeature(pos);
}

void NetherLevelSource::recreateLogicStructuresForChunk(LevelChunk*, int chunkX, int chunkZ)
{
    mNetherBridgeFeature->apply(mLevel, chunkX, chunkZ, NULL);
}
