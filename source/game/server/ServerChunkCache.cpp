#include "game/server/ServerChunkCache.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/chunk/ChunkGenerator.hpp"
#include "game/world/chunk/ChunkStorage.hpp"
#include "game/world/chunk/EmptyLevelChunk.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include <cstdint>
#include <sstream>

ServerChunkCache::ServerChunkCache(ServerLevel* level, ChunkStorage* storage, ChunkGenerator* generator)
    : m_level(level), m_storage(storage), m_generator(generator), m_chunks(0), m_unloadedChunks(), m_emptyChunk(0),
      m_loadedChunks(), m_toSave(), m_xCenter(0), m_zCenter(0), m_width(32), m_depth(32), m_saveAll(false),
      m_boundFunction(0), m_boundData(0)
{
    m_chunks.resize(m_width * m_depth, 0);
    m_unloadedChunks.resize(m_width * m_depth, 0);
    m_emptyChunk = new EmptyLevelChunk(level, 0, 0);
}

ServerChunkCache::~ServerChunkCache()
{
    dropAll();
    delete m_emptyChunk;
    m_emptyChunk = 0;
}

int ServerChunkCache::computeIdx(int x, int z)
{
    return (x + m_depth) * m_width + (z + m_depth);
}

bool ServerChunkCache::inBounds(int x, int z)
{
    if (m_boundFunction)
        return (*m_boundFunction)(m_boundData, x, z);
    return x >= -m_width / 2 && x < m_width / 2 && z >= -m_depth / 2 && z < m_depth / 2;
}

void ServerChunkCache::dontDrop(int x, int z)
{
    if (!inBounds(x, z))
        return;
    LevelChunk* chunk = m_chunks[computeIdx(x, z)];
    if (chunk)
        m_loadedChunks.push_back(chunk);
}

void ServerChunkCache::drop(int x, int z)
{
    if (!inBounds(x, z))
        return;
    const int index = computeIdx(x, z);
    LevelChunk* chunk = m_chunks[index];
    m_chunks[index] = 0;
    if (chunk)
    {
        std::vector<LevelChunk*>::iterator i = m_loadedChunks.begin();
        while (i != m_loadedChunks.end())
        {
            if (*i == chunk)
                i = m_loadedChunks.erase(i);
            else
                ++i;
        }
    }
}

void ServerChunkCache::dropAll()
{
    for (std::size_t i = 0; i < m_chunks.size(); ++i)
        m_chunks[i] = 0;
    m_loadedChunks.clear();
    m_toSave.clear();
}

void ServerChunkCache::invalidateChunksOutsideArea(int minX, int minZ, int maxX, int maxZ)
{
    for (int x = -m_width / 2; x < m_width / 2; ++x)
        for (int z = -m_depth / 2; z < m_depth / 2; ++z)
            if (x < minX || x > maxX || z < minZ || z > maxZ)
                drop(x, z);
}

LevelChunk* ServerChunkCache::getChunkIfLoaded(int x, int z)
{
    return inBounds(x, z) ? m_chunks[computeIdx(x, z)] : 0;
}

LevelChunk* ServerChunkCache::getChunkIfLoadedOrInvalid(int x, int z)
{
    if (!inBounds(x, z))
        return reinterpret_cast<LevelChunk*>(m_storage);
    return m_chunks[computeIdx(x, z)];
}

LevelChunk* ServerChunkCache::getChunkIfHasChunk(int x, int z)
{
    if (!inBounds(x, z))
        return reinterpret_cast<LevelChunk*>(m_storage);
    LevelChunk* chunk = m_chunks[computeIdx(x, z)];
    if (chunk != 0 && !chunk->isInvalid())
        return chunk;
    return 0;
}

LevelChunk* ServerChunkCache::load(int x, int z, LevelChunk* chunk)
{
    if (!m_storage)
        return 0;
    const int index = computeIdx(x, z);
    if (!chunk)
    {
        chunk = m_chunks[index];
        m_chunks[index] = 0;
    }
    if (!chunk || chunk->isInvalid())
    {
        chunk = m_storage->load(m_level, x, z, chunk);
        if (!chunk)
            return 0;
    }
    chunk->setLastSaveTime(m_level->getGameTime());
    if (m_generator)
        m_generator->postProcessLoadedChunk(chunk, x, z);
    return chunk;
}

void ServerChunkCache::updatePostProcessFlag(short index, int x, int z, int dx, int dz, LevelChunk* chunk)
{
    const int neighborX = x + dx;
    const int neighborZ = z + dz;
    if (!hasChunk(neighborX, neighborZ))
        return;
    LevelChunk* neighbor = getChunkIfLoaded(neighborX, neighborZ);
    if (neighbor == reinterpret_cast<LevelChunk*>(m_storage) ||
        (neighbor != 0 && (neighbor->mChunkFlags & 2) != 0))
    {
        chunk->mChunkFlags |= static_cast<unsigned short>(index);
    }
}

void ServerChunkCache::updatePostProcessFlags(int x, int z)
{
    LevelChunk* chunk = getChunk(x, z);
    if (chunk != reinterpret_cast<LevelChunk*>(m_storage))
    {
        updatePostProcessFlag(8, x, z, 0, -1, chunk);
        updatePostProcessFlag(0x10, x, z, -1, -1, chunk);
        updatePostProcessFlag(4, x, z, -1, 0, chunk);
        updatePostProcessFlag(0x20, x, z, -1, 1, chunk);
        updatePostProcessFlag(0x40, x, z, 0, 1, chunk);
        updatePostProcessFlag(0x80, x, z, 1, 1, chunk);
        updatePostProcessFlag(0x100, x, z, 1, 0, chunk);
        updatePostProcessFlag(0x200, x, z, 1, -1, chunk);
        if ((chunk->mChunkFlags & 2) != 0)
        {
            flagPostProcessComplete(4, x + 1, z);
            flagPostProcessComplete(0x10, x + 1, z + 1);
            flagPostProcessComplete(8, x, z + 1);
            flagPostProcessComplete(0x200, x - 1, z + 1);
            flagPostProcessComplete(0x100, x - 1, z);
            flagPostProcessComplete(0x80, x - 1, z - 1);
            flagPostProcessComplete(0x40, x, z - 1);
            flagPostProcessComplete(0x20, x + 1, z - 1);
        }
    }
    flagPostProcessComplete(0, x, z);
}

int ServerChunkCache::updateCacheAndPostProcess(int x, int z, LevelChunk*, LevelChunk* newChunk, bool)
{
    return load(x, z, newChunk) != 0;
}

bool ServerChunkCache::getChunkIfGenerated(int x, int z, bool)
{
    return getChunkIfLoadedOrInvalid(x, z) != 0;
}

LevelChunk* ServerChunkCache::getOrCreateChunk(int x, int z, bool allowEmpty)
{
    LevelChunk* chunk = getChunkIfLoaded(x, z);
    return chunk ? chunk : create(x, z, allowEmpty);
}

int ServerChunkCache::getLoadedChunksCount()
{
    return (int)m_loadedChunks.size();
}

bool ServerChunkCache::hasChunk(int x, int z)
{
    if (!inBounds(x, z))
        return true;
    LevelChunk* chunk = m_chunks[computeIdx(x, z)];
    return chunk != 0 && !chunk->isInvalid();
}

bool ServerChunkCache::isChunkGeneratedAt(int x, int z)
{
    return hasChunk(x, z);
}

std::vector<LevelChunk*>& ServerChunkCache::getLoadedChunkList()
{
    return m_loadedChunks;
}

LevelChunk* ServerChunkCache::create(int x, int z, bool)
{
    return getChunkIfLoaded(x, z);
}

LevelChunk* ServerChunkCache::create(int x, int z)
{
    return create(x, z, false);
}

LevelChunk* ServerChunkCache::getChunk(int x, int z)
{
    return getOrCreateChunk(x, z, false);
}

LevelChunk* ServerChunkCache::getChunkLoadedOrUnloaded(int x, int z)
{
    if (!inBounds(x, z))
        return m_emptyChunk;
    const int index = computeIdx(x, z);
    LevelChunk* chunk = m_chunks[index];
    if (chunk == 0)
        chunk = m_unloadedChunks[index];
    if (chunk != 0 && !chunk->isInvalid())
        return chunk;
    if (m_level->isFindingSpawn() || m_saveAll)
        return create(x, z);
    return m_emptyChunk;
}

void ServerChunkCache::saveEntities(LevelChunk* chunk)
{
    if (m_storage != 0 && chunk != 0)
        m_storage->saveEntities(m_level, chunk);
}
void ServerChunkCache::save(LevelChunk* chunk)
{
    if (m_storage != 0 && chunk != 0)
    {
        chunk->setLastSaveTime(m_level->getGameTime());
        m_storage->save(m_level, chunk);
    }
}
void ServerChunkCache::flagPostProcessComplete(short index, int x, int z)
{
    LevelChunk* chunk = getChunkIfLoaded(x, z);
    if (chunk != 0)
        chunk->flagPostProcessComplete(index);
}
void ServerChunkCache::saveAllEntities()
{
    for (std::deque<LevelChunk*>::iterator i = m_toSave.begin(); i != m_toSave.end(); ++i)
        saveEntities(*i);
    if (m_storage != 0)
        m_storage->flush();
}

void ServerChunkCache::save(bool force, ProgressListener*)
{
    for (std::vector<LevelChunk*>::iterator i = m_loadedChunks.begin(); i != m_loadedChunks.end(); ++i)
    {
        LevelChunk* chunk = *i;
        if (chunk != 0 && (force || !m_toSave.empty()))
        {
            saveEntities(chunk);
            save(chunk);
            chunk->setUnsaved(false);
        }
    }
    if (m_storage != 0)
        m_storage->flush();
}

void ServerChunkCache::tick()
{
    if (m_storage)
        m_toSave.clear();
}

bool ServerChunkCache::shouldSave()
{
    return m_saveAll || !m_toSave.empty();
}

std::wstring ServerChunkCache::gatherStats()
{
    std::wstringstream result;
    result << L"ServerChunkCache_" << m_loadedChunks.size();
    return result.str();
}

std::vector<Biome::MobSpawnerData*> ServerChunkCache::getMobsAt(MobCategory* category, const BlockPos& pos)
{
    return m_generator->getMobsAt(category, pos);
}

BlockPos* ServerChunkCache::findNearestMapFeature(Level* level, const std::wstring& feature,
                                                  const BlockPos& pos, bool takeBlock)
{
    return m_generator->findNearestMapFeature(level, feature, pos, takeBlock);
}

void ServerChunkCache::recreateLogicStructuresForChunk(LevelChunk*, int, int) {}

LevelChunk* ServerChunkCache::getChunkAt(const BlockPos& pos)
{
    return getChunk(pos.x >> 4, pos.z >> 4);
}

unsigned int ServerChunkCache::runSaveThreadProc(void*)
{
    return 0;
}

int ServerChunkCache::getLoadedChunks()
{
    return getLoadedChunksCount();
}

void ServerChunkCache::setOverrideBoundFunction(bool (*function)(void*, int, int), void* data)
{
    m_boundFunction = function;
    m_boundData = data;
}

unsigned int ServerChunkCache::getCache()
{
    return static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(m_chunks.data()));
}
