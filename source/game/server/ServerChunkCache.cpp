#include "game/server/ServerChunkCache.hpp"
#include <sstream>

ServerChunkCache::ServerChunkCache(ServerLevel* level, ChunkStorage* storage, ChunkGenerator* generator)
    : m_level(level), m_storage(storage), m_generator(generator), m_chunks(0), m_loadedChunks(), m_toSave(),
      m_xCenter(0), m_zCenter(0), m_width(32), m_depth(32), m_saveAll(false), m_boundFunction(0), m_boundData(0)
{
    m_chunks.resize(m_width * m_depth, 0);
}

ServerChunkCache::~ServerChunkCache()
{
    dropAll();
}

int ServerChunkCache::computeIdx(int x, int z)
{
    return ((x + m_xCenter) & (m_width - 1)) * m_depth + ((z + m_zCenter) & (m_depth - 1));
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
    return getChunkIfLoaded(x, z);
}

LevelChunk* ServerChunkCache::getChunkIfHasChunk(int x, int z)
{
    return getChunkIfLoaded(x, z);
}

LevelChunk* ServerChunkCache::load(int x, int z, LevelChunk* chunk)
{
    if (inBounds(x, z))
        m_chunks[computeIdx(x, z)] = chunk;
    return chunk;
}

void ServerChunkCache::updatePostProcessFlag(short, int, int, int, int, LevelChunk*) {}
void ServerChunkCache::updatePostProcessFlags(int, int) {}

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
    return getChunkIfLoaded(x, z) != 0;
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
    return getChunkIfLoaded(x, z);
}

void ServerChunkCache::saveEntities(LevelChunk*) {}
void ServerChunkCache::save(LevelChunk*) {}
void ServerChunkCache::flagPostProcessComplete(short, int, int) {}
void ServerChunkCache::saveAllEntities()
{
    for (std::vector<LevelChunk*>::iterator i = m_loadedChunks.begin(); i != m_loadedChunks.end(); ++i)
        saveEntities(*i);
}

void ServerChunkCache::save(bool, ProgressListener*)
{
    saveAllEntities();
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

std::vector<Mob*> ServerChunkCache::getMobsAt(MobCategory*, const BlockPos&)
{
    return std::vector<Mob*>();
}

BlockPos* ServerChunkCache::findNearestMapFeature(Level*, const std::wstring&, const BlockPos&, bool)
{
    return 0;
}

void ServerChunkCache::recreateLogicStructuresForChunk(LevelChunk*, int, int) {}

LevelChunk* ServerChunkCache::getChunkAt(const BlockPos&)
{
    return getChunk(0, 0);
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
    return (unsigned int)m_chunks.size();
}
