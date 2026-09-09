#include "game/world/chunk/ReadOnlyChunkCache.hpp"

#include "game/entity/mob/MobCategory.hpp"
#include "game/network/ProgressListener.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkStorage.hpp"
#include "game/world/chunk/LevelChunk.hpp"

ReadOnlyChunkCache::~ReadOnlyChunkCache()
{
    if (m_chunkCount == 0)
    {
        delete[] m_chunks;
        delete[] m_data;
        return;
    }

    for (int i = 0; i < m_chunkCount; ++i)
    {
        if (m_chunks[i])
            m_chunks[i]->unload(true, true);
    }

    delete[] m_chunks;
    delete[] m_data;
}

bool ReadOnlyChunkCache::hasChunk(int x, int z)
{
    LevelChunk* chunk = m_chunks[((z & 15) << 4) | (x & 15)];
    return chunk != 0 && chunk->isAt(x, z);
}

LevelChunk* ReadOnlyChunkCache::create(int x, int z)
{
    return getChunk(x, z);
}

LevelChunk* ReadOnlyChunkCache::load(int x, int z)
{
    return m_storage->load(m_level, x, z, 0);
}

LevelChunk* ReadOnlyChunkCache::getChunk(int x, int z)
{
    const unsigned int chunkIndex = ((z & 15) << 4) | (x & 15);
    LevelChunk* chunk = m_chunks[chunkIndex];
    if (!chunk)
    {
        chunk = load(x, z);
        if (!chunk)
            return 0;
        m_chunks[chunkIndex] = chunk;
    }
    return chunk;
}

void ReadOnlyChunkCache::postProcess(ChunkSource*, int, int)
{
}

void ReadOnlyChunkCache::save(bool, ProgressListener*)
{
}

void ReadOnlyChunkCache::tick()
{
}

bool ReadOnlyChunkCache::shouldSave()
{
    return false;
}

std::wstring ReadOnlyChunkCache::gatherStats()
{
    return L"ReadOnlyChunkCache";
}

std::vector<Biome::MobSpawnerData*> ReadOnlyChunkCache::getMobsAt(MobCategory*, const BlockPos&)
{
    return std::vector<Biome::MobSpawnerData*>();
}

BlockPos* ReadOnlyChunkCache::findNearestMapFeature(Level*, const std::wstring&, const BlockPos&, bool)
{
    return 0;
}
