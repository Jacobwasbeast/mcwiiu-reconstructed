#include "game/server/ServerChunkCache.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/network/ProgressListener.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/chunk/ChunkGenerator.hpp"
#include "game/world/chunk/ChunkStorage.hpp"
#include "game/world/chunk/EmptyLevelChunk.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"
#include "game/core/Compression.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "platform/wiiu/C4JEvent.hpp"
#include <cstdint>
#include <algorithm>
#include <sstream>

extern void instructionSynchronize();
extern long InterlockedCompareExchangeRelease(volatile long* destination, long exchange, long comparand);

namespace
{
struct ServerChunkCacheSaveContext
{
    ServerChunkCache* cache;
    LevelChunk* chunk;
    bool saveEntities;
    bool useDefaultThreadStorage;
    C4JEvent* notifyEvent;
    ChunkStorage* storage;
};
}

ServerChunkCache::ServerChunkCache(ServerLevel* level, ChunkStorage* storage, ChunkGenerator* generator)
    : m_level(level), m_storage(storage), m_generator(generator), m_chunks(0), m_unloadedChunks(), m_emptyChunk(0),
      m_loadedChunks(), m_toSave(), m_xCenter(0), m_zCenter(0), m_width(generator->mLevelSize), m_halfWidth(generator->mLevelSize / 2), m_saveAll(false),
      m_boundFunction(0), m_boundData(0)
{
    mLevelSize = generator->mLevelSize;
    InitializeCriticalSectionAndSpinCount(&m_mutex, 4000);
    m_chunks.resize(m_width * m_width, 0);
    m_unloadedChunks.resize(m_width * m_width, 0);
    m_emptyChunk = new EmptyLevelChunk(level, 0, 0);
}

ServerChunkCache::~ServerChunkCache()
{
    if (m_storage)
        m_storage->WaitForAll();

    std::vector<LevelChunk*> deleted;
    for (std::vector<LevelChunk*>::iterator i = m_loadedChunks.begin(); i != m_loadedChunks.end(); ++i)
    {
        if (*i && std::find(deleted.begin(), deleted.end(), *i) == deleted.end())
        {
            deleted.push_back(*i);
            delete *i;
        }
    }
    for (std::vector<LevelChunk*>::iterator i = m_unloadedChunks.begin(); i != m_unloadedChunks.end(); ++i)
    {
        if (*i && std::find(deleted.begin(), deleted.end(), *i) == deleted.end())
        {
            deleted.push_back(*i);
            delete *i;
        }
    }
    m_chunks.clear();
    m_unloadedChunks.clear();
    m_loadedChunks.clear();
    m_toSave.clear();
    delete m_generator;
    m_generator = 0;
    delete m_storage;
    m_storage = 0;
    DeleteCriticalSection(&m_mutex);
    delete m_emptyChunk;
    m_emptyChunk = 0;
}

int ServerChunkCache::computeIdx(int x, int z)
{
    return (x + m_halfWidth) * m_width + (z + m_halfWidth);
}

bool ServerChunkCache::inBounds(int x, int z)
{
    if (m_boundFunction)
        return (*m_boundFunction)(m_boundData, x, z);
    return m_level->mDimension->containsChunk(x, z);
}

void ServerChunkCache::dontDrop(int x, int z)
{
    if (!inBounds(x, z))
        return;
    LevelChunk* chunk = m_chunks[computeIdx(x, z)];
    m_toSave.erase(std::remove(m_toSave.begin(), m_toSave.end(), chunk), m_toSave.end());
}

void ServerChunkCache::drop(int x, int z)
{
    if (!inBounds(x, z))
        return;
    const int index = computeIdx(x, z);
    LevelChunk* chunk = m_chunks[index];
    if (chunk)
        m_toSave.push_back(chunk);
}

void ServerChunkCache::invalidateChunksOutsideArea(int minX, int minZ, int maxX, int maxZ)
{
    for (int x = -m_halfWidth; x < m_halfWidth; ++x)
        for (int z = -m_halfWidth; z < m_halfWidth; ++z)
            if (x < minX || x > maxX || z < minZ || z > maxZ)
            {
                const int index = computeIdx(x, z);
                LevelChunk* chunk = m_chunks[index];
                if (chunk)
                    chunk->SetInvalid(true);
                if (m_unloadedChunks[index])
                    m_unloadedChunks[index]->SetInvalid(true);
            }
}

LevelChunk* ServerChunkCache::getChunkIfLoaded(int x, int z)
{
    return inBounds(x, z) ? m_chunks[computeIdx(x, z)] : m_emptyChunk;
}

LevelChunk* ServerChunkCache::getChunkIfLoadedOrInvalid(int x, int z)
{
    if (!inBounds(x, z))
        return m_emptyChunk;
    return m_chunks[computeIdx(x, z)];
}

LevelChunk* ServerChunkCache::getChunkIfHasChunk(int x, int z)
{
    if (!inBounds(x, z))
        return m_emptyChunk;
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
        chunk = m_unloadedChunks[index];
        m_unloadedChunks[index] = 0;
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
    LevelChunk* neighbor = getChunk(neighborX, neighborZ);
    if (neighbor == m_emptyChunk ||
        (neighbor != 0 && (neighbor->mChunkFlags & 2) != 0))
    {
        chunk->mChunkFlags |= static_cast<unsigned short>(index);
    }
}

void ServerChunkCache::updatePostProcessFlags(int x, int z)
{
    LevelChunk* chunk = getChunk(x, z);
    if (chunk != m_emptyChunk)
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

LevelChunk* ServerChunkCache::updateCacheAndPostProcess(int x, int z, LevelChunk* newChunk,
                                                            LevelChunk* oldChunk, bool allowEmpty)
{
    if (!inBounds(x, z))
        return m_emptyChunk;

    const int index = computeIdx(x, z);
    if (InterlockedCompareExchangeRelease(reinterpret_cast<volatile long*>(&m_chunks[index]),
                                          reinterpret_cast<long>(newChunk),
                                          reinterpret_cast<long>(oldChunk)) !=
        reinterpret_cast<long>(oldChunk))
    {
        newChunk->unload(true, true);
        delete newChunk;
        return m_chunks[index];
    }

    EnterCriticalSection(&m_mutex);
    m_generator->lightChunk(newChunk);
    updatePostProcessFlags(x, z);
    if (newChunk != oldChunk)
        m_loadedChunks.push_back(newChunk);

    if (!allowEmpty)
    {
        newChunk->checkPostProcess(this, m_generator);
    }
    else
    {
        if ((newChunk->mChunkFlags & 2) == 0 && hasChunk(x + 1, z + 1) &&
            hasChunk(x, z + 1) && hasChunk(x + 1, z))
            gMinecraftServer->addPostProcessRequest(this, m_generator, x, z);

        if (hasChunk(x - 1, z) && (getChunkIfLoaded(x - 1, z)->mChunkFlags & 2) == 0 &&
            hasChunk(x - 1, z + 1) && hasChunk(x, z + 1))
            gMinecraftServer->addPostProcessRequest(this, m_generator, x - 1, z);

        if (hasChunk(x, z - 1) && (getChunkIfLoaded(x, z - 1)->mChunkFlags & 2) == 0 &&
            hasChunk(x + 1, z - 1) && hasChunk(x, z - 1) && hasChunk(x + 1, z))
            gMinecraftServer->addPostProcessRequest(this, m_generator, x, z - 1);

        if (hasChunk(x - 1, z - 1) && (getChunkIfLoaded(x - 1, z - 1)->mChunkFlags & 2) == 0 &&
            hasChunk(x - 1, z - 1) && hasChunk(x, z - 1) && hasChunk(x - 1, z))
            gMinecraftServer->addPostProcessRequest(this, m_generator, x - 1, z - 1);
    }

    if (hasChunk(x - 1, z) && hasChunk(x - 2, z) && hasChunk(x - 1, z + 1) &&
        hasChunk(x - 1, z - 1))
        newChunk->checkChests(this, x - 1, z);
    if (hasChunk(x, z + 1) && hasChunk(x, z + 2) && hasChunk(x - 1, z + 1) &&
        hasChunk(x + 1, z + 1))
        newChunk->checkChests(this, x, z + 1);
    if (hasChunk(x + 1, z) && hasChunk(x + 2, z) && hasChunk(x + 1, z + 1) &&
        hasChunk(x + 1, z - 1))
        newChunk->checkChests(this, x + 1, z);
    if (hasChunk(x, z - 1) && hasChunk(x, z - 2) && hasChunk(x - 1, z - 1) &&
        hasChunk(x + 1, z - 1))
        newChunk->checkChests(this, x, z - 1);
    if (hasChunk(x - 1, z) && hasChunk(x + 1, z) && hasChunk(x, z - 1) &&
        hasChunk(x, z + 1))
        newChunk->checkChests(this, x, z);

    LeaveCriticalSection(&m_mutex);
    instructionSynchronize();
    return newChunk;
}

LevelChunk* ServerChunkCache::getChunkIfGenerated(int x, int z, bool allowEmpty)
{
    if (!inBounds(x, z))
        return m_emptyChunk;

    const int index = computeIdx(x, z);
    LevelChunk* loaded = m_chunks[index];
    LevelChunk* chunk = getChunkIfLoadedOrInvalid(x, z);
    if (chunk == 0 || chunk == m_emptyChunk || chunk->isInvalid())
    {
        EnterCriticalSection(&m_mutex);
        chunk = load(x, z, chunk);
        if (chunk != 0)
        {
            chunk->load(false);
            LeaveCriticalSection(&m_mutex);
            return updateCacheAndPostProcess(x, z, chunk, loaded, allowEmpty);
        }
        LeaveCriticalSection(&m_mutex);
        return 0;
    }
    return chunk;
}

LevelChunk* ServerChunkCache::getOrCreateChunk(int x, int z, bool allowEmpty)
{
    if (!inBounds(x, z))
        return m_emptyChunk;

    const int index = computeIdx(x, z);
    LevelChunk* oldChunk = m_chunks[index];
    LevelChunk* chunk = getChunkIfGenerated(x, z, allowEmpty);
    if (chunk == 0)
    {
        EnterCriticalSection(&m_mutex);
        const float pixColour = 1.035f;
        PIXBeginNamedEvent(pixColour, "Creating chunk");
        chunk = m_generator->createChunk(x, z);
        PIXEndNamedEvent();
        chunk->load(false);
        LeaveCriticalSection(&m_mutex);
        chunk = updateCacheAndPostProcess(x, z, chunk, oldChunk, allowEmpty);
    }
    return chunk;
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
    if (!inBounds(x, z))
        return false;
    LevelChunk* chunk = m_chunks[computeIdx(x, z)];
    return chunk != 0 && !chunk->isEmpty();
}

std::vector<LevelChunk*>& ServerChunkCache::getLoadedChunkList()
{
    return m_loadedChunks;
}

LevelChunk* ServerChunkCache::create(int x, int z, bool allowEmpty)
{
    if (!inBounds(x, z))
        return m_emptyChunk;

    const int index = computeIdx(x, z);
    LevelChunk* oldChunk = m_chunks[index];
    if (oldChunk != 0 && oldChunk->x == x && oldChunk->z == z
        && !oldChunk->isInvalid())
        return oldChunk;

    EnterCriticalSection(&m_mutex);
    LevelChunk* chunk = load(x, z, oldChunk);
    if (chunk == 0)
    {
        if (m_generator == 0)
            chunk = m_emptyChunk;
        else
            chunk = m_generator->createChunk(x, z);
    }
    if (chunk != 0)
        chunk->load(false);
    LeaveCriticalSection(&m_mutex);
    return updateCacheAndPostProcess(x, z, chunk, oldChunk, allowEmpty);
}

LevelChunk* ServerChunkCache::create(int x, int z)
{
    return create(x, z, false);
}

LevelChunk* ServerChunkCache::getChunk(int x, int z)
{
    if (inBounds(x, z))
    {
        LevelChunk* chunk = m_chunks[computeIdx(x, z)];
        if (chunk != 0 && !chunk->isInvalid())
            return chunk;
        if (m_level->isFindingSpawn() || m_saveAll)
            return create(x, z);
    }
    return m_emptyChunk;
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
    if (m_storage != 0)
    {
        chunk->setLastSaveTime(m_level->getGameTime());
        m_storage->save(m_level, chunk);
    }
}
void ServerChunkCache::flagPostProcessComplete(short index, int x, int z)
{
    if (!hasChunk(x, z))
        return;
    LevelChunk* chunk = m_level->getChunk(x, z);
    if (chunk != 0 && chunk != m_emptyChunk)
        chunk->flagPostProcessComplete(index);
}
void ServerChunkCache::saveAllEntities()
{
    EnterCriticalSection(&m_mutex);
    for (std::vector<LevelChunk*>::iterator i = m_loadedChunks.begin(); i != m_loadedChunks.end(); ++i)
        saveEntities(*i);
    if (m_storage != 0)
        m_storage->flush();
    LeaveCriticalSection(&m_mutex);
}

void ServerChunkCache::save(bool force, ProgressListener* listener)
{
    EnterCriticalSection(&m_mutex);
    int count = 0;
    int completed = 0;
    if (listener)
    {
        for (std::vector<LevelChunk*>::iterator i = m_loadedChunks.begin(); i != m_loadedChunks.end(); ++i)
            if (*i && (*i)->shouldSave(force))
                ++count;
    }
    const int quadrants[4][2] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
    for (int quadrant = 0; quadrant < (force ? 4 : 1); ++quadrant)
    {
        for (std::vector<LevelChunk*>::iterator i = m_loadedChunks.begin(); i != m_loadedChunks.end(); ++i)
        {
            LevelChunk* chunk = *i;
            if (chunk == 0 || (!force && !chunk->shouldSave(false)))
                continue;
            if (force && ((chunk->x < 0) != (quadrants[quadrant][0] < 0) ||
                          (chunk->z < 0) != (quadrants[quadrant][1] < 0)))
                continue;
            if (force)
                saveEntities(chunk);
            if (chunk->shouldSave(force))
            {
                save(chunk);
                chunk->setUnsaved(false);
                if (listener && count != 0 && (++completed % 10) == 0)
                    listener->progressStagePercentage(completed * 100 / count);
            }
        }
    }
    if (m_storage != 0)
        m_storage->flush();
    LeaveCriticalSection(&m_mutex);
}

void ServerChunkCache::tick()
{
    if (!shouldSave())
        return;
    for (int i = 0; i < 50 && !m_toSave.empty(); ++i)
    {
        LevelChunk* chunk = m_toSave.front();
        if (chunk != 0 && !chunk->containsPlayer())
        {
            save(chunk);
            saveEntities(chunk);
            chunk->unload(true, true);
            std::vector<LevelChunk*>::iterator loaded = std::find(m_loadedChunks.begin(), m_loadedChunks.end(), chunk);
            if (loaded != m_loadedChunks.end())
                m_loadedChunks.erase(loaded);
            const int index = computeIdx(chunk->x, chunk->z);
            m_unloadedChunks[index] = chunk;
            m_chunks[index] = 0;
        }
        m_toSave.pop_front();
    }
    if (m_storage)
        m_storage->tick();
}

bool ServerChunkCache::shouldSave()
{
    return !m_saveAll;
}

std::wstring ServerChunkCache::gatherStats()
{
    return L"ServerChunkCache";
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

unsigned int ServerChunkCache::runSaveThreadProc(void* data)
{
    ServerChunkCacheSaveContext* context = static_cast<ServerChunkCacheSaveContext*>(data);
    if (context == 0)
        return 0;

    if (!context->useDefaultThreadStorage)
    {
        Compression::CreateNewThreadStorage();
        OldChunkStorage::CreateNewThreadStorage();
        if (context->storage != 0)
            context->storage->WaitIfTooManyQueuedChunks();
    }
    else
    {
        Compression::UseDefaultThreadStorage();
        OldChunkStorage::UseDefaultThreadStorage();
        if (context->storage != 0)
            context->storage->WaitIfTooManyQueuedChunks();
    }

    if (context->chunk != 0)
    {
        const float pixColour = 1.035f;
        do
        {
            if (context->saveEntities)
            {
                PIXBeginNamedEvent(pixColour, "Saving entities");
                context->cache->saveEntities(context->chunk);
                PIXEndNamedEvent();
            }
            PIXBeginNamedEvent(pixColour, "Saving chunk");
            context->cache->save(context->chunk);
            context->chunk->setUnsaved(false);
            PIXEndNamedEvent();
            PIXBeginNamedEvent(pixColour, "Notifying and waiting for next chunk");
            if (context->notifyEvent != 0)
                context->notifyEvent->Set();
            if (context->storage != 0)
                context->storage->WaitIfTooManyQueuedChunks();
            PIXEndNamedEvent();
        }
        while (context->chunk != 0);
    }

    if (!context->useDefaultThreadStorage)
    {
        Compression::ReleaseThreadStorage();
        OldChunkStorage::ReleaseThreadStorage();
    }
    instructionSynchronize();
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

LevelChunk** ServerChunkCache::getCache()
{
    return m_chunks.data();
}
