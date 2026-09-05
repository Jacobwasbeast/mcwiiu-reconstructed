#include "game/world/NetherFlatLevelSource.hpp"

#include <cmath>

#include "game/core/CMinecraftApp.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/HellFireFeature.hpp"
#include "game/world/generation/LightGemFeature.hpp"

void* XPhysicalAlloc(unsigned long size, unsigned long upperAddress, unsigned long alignment, unsigned int protect);
void XPhysicalFree(void* address);
void XMemSet128(void* destination, int value, unsigned int count);

NetherFlatLevelSource::NetherFlatLevelSource(Level* level, bool generateStructures, long long seed)
    : mLevelHeight(static_cast<int>(std::ceil((static_cast<float>(level->getHeight()) - 176.0f) /
                                             (static_cast<float>(level->getMaxBuildHeight()) - 176.0f)))),
      mRandom(new Random()),
      mPopulationRandom(new Random(seed)),
      mGenerateStructures(generateStructures),
      mLevel(level)
{
}

NetherFlatLevelSource::~NetherFlatLevelSource()
{
    delete mRandom;
    delete mPopulationRandom;
}

void NetherFlatLevelSource::prepareHeights(int, int, ChunkPrimer* primer)
{
    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            for (int y = 0; y < 16; ++y)
            {
                const BlockState* state = Blocks::AIR->defaultBlockState();
                if (y < 7)
                {
                    state = Blocks::netherrack->defaultBlockState();
                }
                primer->setState((x << 11) | (z << 7) | y, state);
            }
        }
    }
}

void NetherFlatLevelSource::buildSurfaces(int chunkX, int chunkZ, ChunkPrimer* primer)
{
    for (int z = 0; z < 16; ++z)
    {
        for (int x = 0; x < 16; ++x)
        {
            int blockIndex = (x << 11) | (z << 7) | 127;
            for (int y = 127; y >= 0; --y, --blockIndex)
            {
                const int depth = mLevelHeight / 2;
                bool placed = false;

                if (chunkX <= -depth)
                {
                    if (x - mRandom->nextInt(4) <= 0 || chunkX < -depth)
                    {
                        primer->setState(blockIndex, Blocks::bedrock->defaultBlockState());
                        placed = true;
                    }
                }

                if (chunkZ <= -depth)
                {
                    if (z - mRandom->nextInt(4) <= 0 || chunkZ < -depth)
                    {
                        primer->setState(blockIndex, Blocks::bedrock->defaultBlockState());
                        placed = true;
                    }
                }

                if (depth - 1 <= chunkX)
                {
                    if (x + mRandom->nextInt(4) >= 15 || depth < chunkX)
                    {
                        primer->setState(blockIndex, Blocks::bedrock->defaultBlockState());
                        placed = true;
                    }
                }

                if (chunkZ < depth - 1)
                {
                    if (!placed)
                    {
                        const int topDepth = mRandom->nextInt(5);
                        if (y >= 127 - topDepth || y <= mRandom->nextInt(5))
                        {
                            primer->setState(blockIndex, Blocks::bedrock->defaultBlockState());
                        }
                    }
                }
                else if (z + mRandom->nextInt(4) >= 15 || depth < chunkZ)
                {
                    primer->setState(blockIndex, Blocks::bedrock->defaultBlockState());
                }
                else if (!placed)
                {
                    const int topDepth = mRandom->nextInt(5);
                    if (y >= 127 - topDepth || y <= mRandom->nextInt(5))
                    {
                        primer->setState(blockIndex, Blocks::bedrock->defaultBlockState());
                    }
                }
            }
        }
    }
}

LevelChunk* NetherFlatLevelSource::createChunk(int chunkX, int chunkZ)
{
    mRandom->setSeed(static_cast<long long>(chunkX) * 341873128712LL + static_cast<long long>(chunkZ) * 132897987541LL);
    unsigned short* elementIds = static_cast<unsigned short*>(XPhysicalAlloc(65536, 0xFFFFFFFF, 0x1000, 4));
    XMemSet128(elementIds, 0, 65536);
    arrayWithLength<unsigned short> elements;
    elements.data = elementIds;
    elements.length = 32768;
    ChunkPrimer primer(false, elements);
    prepareHeights(chunkX, chunkZ, &primer);
    buildSurfaces(chunkX, chunkZ, &primer);
    LevelChunk* chunk = new LevelChunk(mLevel, &primer, chunkX, chunkZ);
    XPhysicalFree(elementIds);
    return chunk;
}

void NetherFlatLevelSource::lightChunk(LevelChunk* chunk)
{
    chunk->recalcHeightmap();
}

void NetherFlatLevelSource::postProcess(int chunkX, int chunkZ)
{
    const int originX = chunkX * 16;
    const int originZ = chunkZ * 16;
    const long long seed = mLevel->getSeed();
    mPopulationRandom->setSeed(seed);
    const long long xSeed = mPopulationRandom->nextLong() / 2LL * 2LL + 1LL;
    const long long zSeed = mPopulationRandom->nextLong() / 2LL * 2LL + 1LL;
    mPopulationRandom->setSeed((static_cast<long long>(chunkX) * xSeed + static_cast<long long>(chunkZ) * zSeed) ^ seed);

    int count = mPopulationRandom->nextInt(10);
    count = mPopulationRandom->nextInt(count + 1) + 1;
    for (int i = 0; i < count; ++i)
    {
        BlockPos pos(originX + mPopulationRandom->nextInt(16) + 8,
                     mPopulationRandom->nextInt(120) + 4,
                     originZ + mPopulationRandom->nextInt(16) + 8);
        HellFireFeature feature;
        feature.place(mLevel, *mPopulationRandom, pos);
    }

    count = mPopulationRandom->nextInt(10);
    count = mPopulationRandom->nextInt(count + 1);
    for (int i = 0; i < count; ++i)
    {
        BlockPos pos(originX + mPopulationRandom->nextInt(16) + 8,
                     mPopulationRandom->nextInt(120) + 4,
                     originZ + mPopulationRandom->nextInt(16) + 8);
        LightGemFeature feature;
        feature.place(mLevel, *mPopulationRandom, pos);
    }

    g_pApp->processSchematics(mLevel->getChunk(chunkX, chunkZ));
}

void NetherFlatLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
}

std::vector<Biome::MobSpawnerData*> NetherFlatLevelSource::getMobsAt(MobCategory* category, const BlockPos& pos)
{
    Biome* biome = mLevel->getBiome(pos);
    return *biome->getMobs(category);
}

BlockPos* NetherFlatLevelSource::findNearestMapFeature(Level*, const std::wstring&, const BlockPos&, bool)
{
    return NULL;
}

bool NetherFlatLevelSource::isPosInFeature(Level*, const std::wstring&, const BlockPos&)
{
    return false;
}

void NetherFlatLevelSource::recreateLogicStructuresForChunk(LevelChunk*, int, int)
{
}
