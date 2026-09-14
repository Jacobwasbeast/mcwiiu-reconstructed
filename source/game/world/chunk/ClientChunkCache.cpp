#include "game/world/chunk/ClientChunkCache.hpp"

#include "game/client/render/GameRenderer.hpp"
#include "game/core/Compression.hpp"
#include "game/core/System.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ProgressListener.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/ChunkStorage.hpp"
#include "game/world/chunk/EmptyLevelChunk.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

ClientChunkCache::QueuedAction::QueuedAction()
    : action(0), x(0), z(0), pos(0, 0), changes(0), packet()
{
}

ClientChunkCache::QueuedAction::QueuedAction(const boost::shared_ptr<ClientboundBlockEntityDataPacket>& value)
    : action(1), x(0), z(0), pos(0, 0), changes(0), packet(value)
{
    if (value)
    {
        pos = ChunkPos(value->getPos());
        x = pos.x;
        z = pos.z;
    }
}

ClientChunkCache::QueuedAction::QueuedAction(int chunkX, int chunkZ,
                                              const std::vector<LevelChunk::BlockChange>* value)
    : action(0), x(chunkX), z(chunkZ), pos(chunkX, chunkZ), changes(value), packet()
{
}

ClientChunkCache::QueuedAction& ClientChunkCache::QueuedAction::operator=(const QueuedAction& other)
{
    if (this != &other)
    {
        action = other.action;
        x = other.x;
        z = other.z;
        pos = other.pos;
        changes = other.changes;
        packet = other.packet;
    }
    return *this;
}

ClientChunkCache::ClientChunkCache(Level* level, ChunkStorage* storage)
    : m_emptyChunk(0), m_fallbackChunk(0), m_storage(storage), m_level(level),
      m_spawnPos(0, 0), m_chunkRadius(0), m_initialized(false), m_stopThread(false),
      m_loadedChunks(), m_queuedActions(), m_chunks(), m_received(), m_unloadQueue(), m_cache(0),
      m_storageThread(0), m_reloadPending(false), m_reloadX(0), m_reloadZ(0), m_reloadChanges(0), m_dimension(0)
{
    MultiPlayerChunkCacheInit(level, storage);
}

ClientChunkCache::ClientChunkCache(Level* level)
    : m_emptyChunk(0), m_fallbackChunk(0), m_storage(0), m_level(level),
      m_spawnPos(0, 0), m_chunkRadius(0), m_initialized(false), m_stopThread(false),
      m_loadedChunks(), m_queuedActions(), m_chunks(), m_received(), m_unloadQueue(), m_cache(0),
      m_storageThread(0), m_reloadPending(false), m_reloadX(0), m_reloadZ(0), m_reloadChanges(0), m_dimension(0)
{
    MultiPlayerChunkCacheInit(level, 0);
}

ClientChunkCache::~ClientChunkCache()
{
    for (std::vector<LevelChunk*>::iterator it = m_chunks.begin(); it != m_chunks.end(); ++it)
    {
        if (*it && *it != m_emptyChunk)
            delete *it;
    }
    delete m_emptyChunk;
}

LevelChunk* ClientChunkCache::load(int x, int z, LevelChunk* chunk)
{
    if (!m_storage)
        return 0;
    LevelChunk* loaded = m_storage->load(m_level, x, z, chunk);
    if (loaded)
        loaded->setLastSaveTime(m_level->getGameTime());
    return loaded;
}

void ClientChunkCache::reLoad(int x, int z, const std::vector<LevelChunk::BlockChange>* changes)
{
    const unsigned int index = static_cast<unsigned int>((z + m_chunkRadius) * (2 * m_chunkRadius) + x + m_chunkRadius);
    if (index >= m_chunks.size())
        return;
    EnterCriticalSection(&m_saveDataCriticalSection);
    LevelChunk* replacement = load(x, z, 0);
    if (!replacement)
    {
        LeaveCriticalSection(&m_saveDataCriticalSection);
        return;
    }
    LevelChunk* old = m_chunks[index];
    m_chunks[index] = replacement;
    if (old)
    {
        old->unload(true, false);
        replacement->swapEntities(old);
        for (std::vector<LevelChunk*>::iterator it = m_loadedChunks.begin(); it != m_loadedChunks.end(); ++it)
        {
            if (*it == old)
            {
                m_loadedChunks.erase(it);
                break;
            }
        }
        GameRenderer::AddForDelete(old);
        GameRenderer::FinishedReassigning();
    }
    replacement->load(true);
    replacement->recalcHeightmapOnly();
    if (m_level)
        m_level->setBlocksDirty(BlockPos(x * 16, 0, z * 16), BlockPos(x * 16 + 15, 255, z * 16 + 15));
    if (index < m_received.size())
        m_received[index] = 1;
    if (changes)
    {
        for (std::vector<LevelChunk::BlockChange>::const_iterator it = changes->begin(); it != changes->end(); ++it)
        {
            replacement->setBlock(it->GetPos(), it->GetState());
            replacement->setExtraBlock(it->GetPos(), it->GetExtraState());
        }
    }
    m_loadedChunks.push_back(replacement);
    LeaveCriticalSection(&m_saveDataCriticalSection);
}

int ClientChunkCache::StorageThreadProc(void* argument)
{
    ClientChunkCache* cache = static_cast<ClientChunkCache*>(argument);
    Compression::UseDefaultThreadStorage();
    OldChunkStorage::UseDefaultThreadStorage();
    AABB::CreateNewThreadStorage();
    Vec3::CreateNewThreadStorage();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_ClientChunkStorage);
    while (!cache->m_stopThread)
    {
        for (;;)
        {
            EnterCriticalSection(&cache->m_threadCriticalSection);
            if (!cache->m_reloadPending)
                break;
            const int x = cache->m_reloadX;
            const int z = cache->m_reloadZ;
            const std::vector<LevelChunk::BlockChange>* changes = cache->m_reloadChanges;
            LeaveCriticalSection(&cache->m_threadCriticalSection);
            cache->reLoad(x, z, changes);
            delete changes;
            cache->m_reloadChanges = 0;
            cache->m_reloadPending = false;
            if (cache->m_stopThread)
            {
                AABB::ReleaseThreadStorage();
                Vec3::ReleaseThreadStorage();
                ElementStorage::ReleaseThreadStorage();
                return 0;
            }
        }
        LeaveCriticalSection(&cache->m_threadCriticalSection);
        C4JThreadImpl::Sleep(1);
    }
    AABB::ReleaseThreadStorage();
    Vec3::ReleaseThreadStorage();
    ElementStorage::ReleaseThreadStorage();
    return 0;
}

void ClientChunkCache::MultiPlayerChunkCacheInit(Level* level, ChunkStorage* storage)
{
    m_level = level;
    m_dimension = level ? level->mDimension : 0;
    m_storage = storage;
    m_chunkRadius = 8;
    m_emptyChunk = new EmptyLevelChunk(level, 0, 0);
    m_chunks.assign(256, static_cast<LevelChunk*>(0));
    m_received.assign(256, static_cast<unsigned char>(0));
    if (storage)
    {
        InitializeCriticalSection(&m_threadCriticalSection);
        m_storageThread = new C4JThreadImpl(&StorageThreadProc, this, "MultiPlayerChunkCache", 0x100000);
        m_storageThread->SetProcessor(0);
        m_storageThread->Run();
    }
    InitializeCriticalSectionAndSpinCount(&m_saveDataCriticalSection, 4000);
    m_initialized = true;
}

uint64_t ClientChunkCache::BlockUntilSafeToChangeSaveData(bool block)
{
    if (!m_storage)
        return 0;
    while (!m_queuedActions.empty())
    {
        QueuedAction action = m_queuedActions.front();
        m_queuedActions.erase(m_queuedActions.begin());
        if (action.action == 0)
        {
            reLoad(action.x, action.z, action.changes);
            delete action.changes;
        }
        if (!block)
            break;
    }
    return static_cast<uint64_t>(m_queuedActions.size());
}

LevelChunk* ClientChunkCache::getChunkIfLoaded(int x, int z)
{
    const int diameter = 2 * m_chunkRadius;
    const int localX = x + m_chunkRadius;
    const int localZ = z + m_chunkRadius;
    if (localX < 0 || localX >= diameter || localZ < 0 || localZ >= diameter)
        return 0;
    const unsigned int index = static_cast<unsigned int>(localZ * diameter + localX);
    if (index >= m_chunks.size())
        return 0;
    LevelChunk* chunk = m_chunks[index];
    return chunk && chunk->isAt(x, z) ? chunk : 0;
}

LevelChunk* ClientChunkCache::getOrCreateChunk(int x, int z, bool allowEmpty)
{
    LevelChunk* chunk = getChunkIfLoaded(x, z);
    if (chunk)
        return chunk;
    if (allowEmpty && m_emptyChunk)
        return m_emptyChunk;
    return create(x, z);
}

int ClientChunkCache::getLoadedChunksCount()
{
    return static_cast<int>(m_loadedChunks.size());
}

bool ClientChunkCache::isChunkGeneratedAt(int x, int z)
{
    return getChunkIfLoaded(x, z) != 0;
}

LevelChunk* ClientChunkCache::getChunkIfHasChunk(int x, int z)
{
    return getChunk(x, z);
}

bool ClientChunkCache::hasChunk(int x, int z)
{
    return reallyHasChunk(x, z);
}

bool ClientChunkCache::reallyHasChunk(int x, int z)
{
    const int diameter = 2 * m_chunkRadius;
    const unsigned int index = static_cast<unsigned int>((z + m_chunkRadius) * diameter + x + m_chunkRadius);
    if (index >= m_chunks.size() || !m_chunks[index])
        return false;
    return index < m_received.size() && m_received[index] != 0;
}

void ClientChunkCache::unqueueForReLoad(int x, int z)
{
    EnterCriticalSection(&m_saveDataCriticalSection);
    for (std::vector<QueuedAction>::iterator it = m_queuedActions.begin(); it != m_queuedActions.end(); ++it)
    {
        if (it->action == 0 && it->x == x && it->z == z)
        {
            m_queuedActions.erase(it);
            break;
        }
    }
    LeaveCriticalSection(&m_saveDataCriticalSection);
}

void ClientChunkCache::drop(int x, int z, bool force)
{
    LevelChunk* chunk = getChunkIfLoaded(x, z);
    if (!chunk)
        return;
    if (!force)
    {
        m_loadedChunks.push_back(chunk);
        return;
    }
    const int diameter = 2 * m_chunkRadius;
    const int localX = x + m_chunkRadius;
    const int localZ = z + m_chunkRadius;
    if (localX < 0 || localX >= diameter || localZ < 0 || localZ >= diameter)
        return;
    const unsigned int index = static_cast<unsigned int>(localZ * diameter + localX);
    if (index < m_chunks.size())
        m_chunks[index] = 0;
    for (std::vector<LevelChunk*>::iterator it = m_loadedChunks.begin(); it != m_loadedChunks.end(); ++it)
    {
        if (*it == chunk)
        {
            m_loadedChunks.erase(it);
            break;
        }
    }
    delete chunk;
}

uint64_t ClientChunkCache::dropOutsideArea(int minX, int minZ, int maxX, int maxZ, bool force)
{
    for (int x = -m_chunkRadius; x < m_chunkRadius; ++x)
        for (int z = -m_chunkRadius; z < m_chunkRadius; ++z)
            if (x < minX || x > maxX || z < minZ || z > maxZ)
                drop(x, z, force);
    return 0;
}

LevelChunk* ClientChunkCache::create(int x, int z)
{
    const int diameter = 2 * m_chunkRadius;
    const int localX = x + m_chunkRadius;
    const int localZ = z + m_chunkRadius;
    if (localX < 0 || localX >= diameter || localZ < 0 || localZ >= diameter)
        return 0;
    const unsigned int index = static_cast<unsigned int>(localZ * diameter + localX);
    if (index >= m_chunks.size())
        m_chunks.resize(index + 1, static_cast<LevelChunk*>(0));
    LevelChunk* chunk = m_chunks[index];
    if (!chunk)
    {
        EnterCriticalSection(&m_saveDataCriticalSection);
        ChunkPrimer primer(true, arrayWithLength<unsigned short>());
        chunk = new LevelChunk(m_level, &primer, x, z);
        chunk->setSkyLightDataAllBright();
        m_chunks[index] = chunk;
        if (index < m_received.size())
            m_received[index] = 1;
        m_loadedChunks.push_back(chunk);
        LeaveCriticalSection(&m_saveDataCriticalSection);
    }
    if (CGameNetworkManager::sInstance.IsHost() && !chunk->isEmpty())
    {
        chunk->recalcHeightmapOnly();
        if (chunk->hasLightingBeenDropped())
            chunk->reSyncLighting();
    }
    return chunk;
}

LevelChunk* ClientChunkCache::getChunk(int x, int z)
{
    LevelChunk* chunk = getChunkIfLoaded(x, z);
    if (chunk)
        return chunk;
    return m_fallbackChunk ? m_fallbackChunk : m_emptyChunk;
}

LevelChunk* ClientChunkCache::getChunkAt(const BlockPos& pos)
{
    return getChunk(pos.x >> 4, pos.z >> 4);
}

void ClientChunkCache::save(bool, ProgressListener*)
{
}

void ClientChunkCache::tick()
{
    EnterCriticalSection(&m_saveDataCriticalSection);
    for (int processed = 0; processed < 100 && !m_unloadQueue.empty(); ++processed)
    {
        LevelChunk* chunk = m_unloadQueue.front();
        m_unloadQueue.pop_front();
        for (std::vector<LevelChunk*>::iterator it = m_loadedChunks.begin(); it != m_loadedChunks.end(); ++it)
        {
            if (*it == chunk)
            {
                m_loadedChunks.erase(it);
                break;
            }
        }
    }
    const uint64_t start = System::processTimeInMilliSecs();
    std::vector<LevelChunk*>::iterator it = m_loadedChunks.begin();
    for (; it != m_loadedChunks.end(); ++it)
    {
        if (static_cast<int64_t>(System::processTimeInMilliSecs() - start) > 4)
            break;
        (*it)->tick(false);
    }
    for (; it != m_loadedChunks.end(); ++it)
        (*it)->tick(true);
    LeaveCriticalSection(&m_saveDataCriticalSection);
}

bool ClientChunkCache::shouldSave()
{
    return false;
}

std::wstring ClientChunkCache::gatherStats()
{
    return std::wstring(L"MultiplayerChunkCache_") + std::wstring();
}

std::vector<Biome::MobSpawnerData*> ClientChunkCache::getMobsAt(MobCategory*, const BlockPos&)
{
    return std::vector<Biome::MobSpawnerData*>();
}

BlockPos* ClientChunkCache::findNearestMapFeature(Level*, const std::wstring&, const BlockPos&, bool)
{
    return 0;
}

int ClientChunkCache::getLoadedChunks()
{
    return static_cast<int>(m_loadedChunks.size());
}

void ClientChunkCache::recreateLogicStructuresForChunk(LevelChunk*, int, int)
{
}

void ClientChunkCache::dataReceived(int x, int z)
{
    const int diameter = 2 * m_chunkRadius;
    const int localX = x + m_chunkRadius;
    const int localZ = z + m_chunkRadius;
    if (localX < 0 || localX >= diameter || localZ < 0 || localZ >= diameter)
        return;
    const unsigned int index = static_cast<unsigned int>(localZ * diameter + localX);
    if (index >= m_received.size())
        m_received.resize(index + 1, static_cast<unsigned char>(0));
    m_received[index] = 1;
}

void ClientChunkCache::queueBlockEntityDataPacket(const boost::shared_ptr<ClientboundBlockEntityDataPacket>& packet)
{
    m_queuedActions.push_back(QueuedAction(packet));
}

void ClientChunkCache::queueForReLoad(int x, int z, const std::vector<LevelChunk::BlockChange>* changes, bool replace)
{
    if (replace)
        unqueueForReLoad(x, z);
    m_queuedActions.push_back(QueuedAction(x, z, changes));
}

bool ClientChunkCache::mustQueueBlockEntityDataPackets()
{
    return !m_queuedActions.empty();
}

unsigned int ClientChunkCache::getCache()
{
    return m_cache;
}
