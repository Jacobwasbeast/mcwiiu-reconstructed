#include "game/world/NetherFlatLevelSource.hpp"

#include <cmath>

#include "game/core/CMinecraftApp.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/HellFireFeature.hpp"
#include "game/world/generation/LightGemFeature.hpp"

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
            for (int y = 0; y < 24; ++y)
            {
                const BlockState* state = Blocks::netherrack->defaultBlockState();
                if (y < 4)
                {
                    state = Blocks::bedrock->defaultBlockState();
                }
                primer->setState(x, y, z, state);
            }
        }
    }
}

void NetherFlatLevelSource::buildSurfaces(int chunkX, int chunkZ, ChunkPrimer* primer)
{
    int xOffset = 0;
    int zOffset = 0;
    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            int y = 127;
            for (int index = 0; index < 128; ++index)
            {
                const int depth = mLevelHeight / 2;
                bool changed = false;
                int surface = -depth;
                if (chunkX <= surface)
                {
                    int random = mRandom->nextInt(4);
                    if (x - random > 0)
                    {
                        surface = -depth;
                        if (surface <= chunkX)
                        {
                            goto next_surface;
                        }
                    }
                    primer->setState(x, y, z, Blocks::AIR->defaultBlockState());
                    changed = true;
                }
            next_surface:
                if (chunkZ <= surface)
                {
                    int random = mRandom->nextInt(4);
                    if (z - random > 0)
                    {
                        surface = -depth;
                        if (surface <= chunkZ)
                        {
                            goto finish_surface;
                        }
                    }
                    primer->setState(x, y, z, Blocks::AIR->defaultBlockState());
                    changed = true;
                }
            finish_surface:
                if (surface - 1 <= chunkX)
                {
                    int random = mRandom->nextInt(4);
                    if (x + random < 15 && depth < chunkX)
                    {
                        goto place_surface;
                    }
                    primer->setState(x, y, z, Blocks::AIR->defaultBlockState());
                    changed = true;
                }
                if (chunkZ < surface - 1)
                {
                    if (!changed)
                    {
                        int random = mRandom->nextInt(5);
                        if (y < 127 - random)
                        {
                            random = mRandom->nextInt(5);
                            if (random < y)
                            {
                                goto next_surface;
                            }
                        }
                    }
                    primer->setState(x, y, z, Blocks::AIR->defaultBlockState());
                }
            place_surface:
                --y;
            }
            zOffset += 128;
        }
        xOffset += 2048;
    }
}

LevelChunk* NetherFlatLevelSource::createChunk(int chunkX, int chunkZ)
{
    mRandom->setSeed(static_cast<long long>(chunkX) * 341873128712LL + static_cast<long long>(chunkZ) * 132897987541LL);
    arrayWithLength<unsigned short> elements(65536, true);
    ChunkPrimer primer(false, elements);
    prepareHeights(chunkX, chunkZ, &primer);
    buildSurfaces(chunkX, chunkZ, &primer);
    LevelChunk* chunk = new LevelChunk(mLevel, &primer, chunkX, chunkZ);
    chunk->recalcHeightmap();
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

    if (mGenerateStructures)
    {
        int count = mPopulationRandom->nextInt(10);
        count = mPopulationRandom->nextInt(count + 1);
        for (int i = 0; i < count; ++i)
        {
            BlockPos pos(originX + mPopulationRandom->nextInt(16) + 8,
                         mPopulationRandom->nextInt(120) + 4,
                         originZ + mPopulationRandom->nextInt(16) + 8);
            HellFireFeature feature;
            feature.place(mLevel, *mPopulationRandom, pos);
        }
    }

    int count = mPopulationRandom->nextInt(10);
    count = mPopulationRandom->nextInt(count + 1);
    for (int i = 0; i < count; ++i)
    {
        BlockPos pos(originX + mPopulationRandom->nextInt(16) + 8,
                     mPopulationRandom->nextInt(120) + 4,
                     originZ + mPopulationRandom->nextInt(16) + 8);
        LightGemFeature feature;
        feature.place(mLevel, *mPopulationRandom, pos);
    }

    CMinecraftApp::sInstance.processSchematics(mLevel->getChunk(chunkX, chunkZ));
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
