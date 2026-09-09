#ifndef GAME_WORLD_CHUNK_CLIENTCHUNKCACHE_HPP
#define GAME_WORLD_CHUNK_CLIENTCHUNKCACHE_HPP

#include <string>
#include <vector>
#include <stdint.h>
#include <boost/shared_ptr.hpp>

#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"

class ChunkStorage;
class Level;

class ClientChunkCache : public ChunkSource
{
public:
    class QueuedAction
    {
    public:
        QueuedAction(const boost::shared_ptr<ClientboundBlockEntityDataPacket>& packet);
        QueuedAction(int x, int z, const std::vector<LevelChunk::BlockChange>* changes);
        QueuedAction();
        QueuedAction& operator=(const QueuedAction& other);

        int action;
        int x;
        int z;
        ChunkPos pos;
        const std::vector<LevelChunk::BlockChange>* changes;
        boost::shared_ptr<ClientboundBlockEntityDataPacket> packet;
    };

    ClientChunkCache(Level* level, ChunkStorage* storage);
    ClientChunkCache(Level* level);
    virtual ~ClientChunkCache();

    LevelChunk* load(int x, int z, LevelChunk* chunk);
    void reLoad(int x, int z, const std::vector<LevelChunk::BlockChange>* changes);
    static int StorageThreadProc(void* argument);
    void MultiPlayerChunkCacheInit(Level* level, ChunkStorage* storage);
    uint64_t BlockUntilSafeToChangeSaveData(bool block);

    virtual LevelChunk* getChunkIfLoaded(int x, int z);
    virtual LevelChunk* getOrCreateChunk(int x, int z, bool allowEmpty);
    virtual int getLoadedChunksCount();
    virtual bool isChunkGeneratedAt(int x, int z);
    virtual LevelChunk* getChunkIfHasChunk(int x, int z);
    virtual bool hasChunk(int x, int z);
    virtual bool reallyHasChunk(int x, int z);
    void unqueueForReLoad(int x, int z);
    void drop(int x, int z, bool force);
    uint64_t dropOutsideArea(int minX, int minZ, int maxX, int maxZ, bool force);
    virtual LevelChunk* create(int x, int z);
    virtual LevelChunk* getChunk(int x, int z);
    virtual LevelChunk* getChunkAt(const BlockPos& pos);
    virtual void save(bool force, ProgressListener* listener);
    virtual void tick();
    virtual bool shouldSave();
    virtual std::wstring gatherStats();
    virtual std::vector<Biome::MobSpawnerData*> getMobsAt(MobCategory* category, const BlockPos& pos);
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature,
                                             const BlockPos& pos, bool takeBlock);
    virtual int getLoadedChunks();
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int x, int z);
    virtual void dataReceived(int x, int z);
    void queueBlockEntityDataPacket(const boost::shared_ptr<ClientboundBlockEntityDataPacket>& packet);
    void queueForReLoad(int x, int z, const std::vector<LevelChunk::BlockChange>* changes, bool replace);
    bool mustQueueBlockEntityDataPackets();
    virtual unsigned int getCache();

private:
    LevelChunk* m_emptyChunk;
    LevelChunk* m_fallbackChunk;
    ChunkStorage* m_storage;
    Level* m_level;
    ChunkPos m_spawnPos;
    int m_chunkRadius;
    bool m_initialized;
    bool m_stopThread;
    std::vector<LevelChunk*> m_loadedChunks;
    std::vector<QueuedAction> m_queuedActions;
    std::vector<LevelChunk*> m_chunks;
    std::vector<unsigned char> m_received;
    unsigned int m_cache;
};

#endif
