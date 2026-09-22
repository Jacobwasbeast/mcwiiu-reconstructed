#ifndef GAME_SERVER_SERVERCHUNKCache_HPP
#define GAME_SERVER_SERVERCHUNKCache_HPP

#include "game/world/chunk/ChunkSource.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/world/biome/Biome.hpp"
#include <deque>
#include <vector>
#include <string>

class ServerLevel;
class ChunkStorage;
class ChunkGenerator;
class LevelChunk;
class BlockPos;
class Level;
class MobCategory;
class ProgressListener;

class ServerChunkCache : public ChunkSource
{
public:
    ServerChunkCache(ServerLevel* level, ChunkStorage* storage, ChunkGenerator* generator);
    virtual ~ServerChunkCache();
    void dontDrop(int x, int z);
    int computeIdx(int x, int z);
    bool inBounds(int x, int z);
    void drop(int x, int z);
    void dropAll();
    void invalidateChunksOutsideArea(int minX, int minZ, int maxX, int maxZ);
    virtual LevelChunk* getChunkIfLoaded(int x, int z);
    LevelChunk* getChunkIfLoadedOrInvalid(int x, int z);
    virtual LevelChunk* getChunkIfHasChunk(int x, int z);
    LevelChunk* load(int x, int z, LevelChunk* chunk);
    void updatePostProcessFlag(short index, int x, int z, int dx, int dz, LevelChunk* chunk);
    void updatePostProcessFlags(int x, int z);
    LevelChunk* updateCacheAndPostProcess(int x, int z, LevelChunk* newChunk, LevelChunk* oldChunk, bool allowEmpty);
    LevelChunk* getChunkIfGenerated(int x, int z, bool allowEmpty);
    virtual LevelChunk* getOrCreateChunk(int x, int z, bool allowEmpty);
    virtual int getLoadedChunksCount();
    virtual bool hasChunk(int x, int z);
    virtual bool isChunkGeneratedAt(int x, int z);
    std::vector<LevelChunk*>& getLoadedChunkList();
    LevelChunk* create(int x, int z, bool allowEmpty);
    virtual LevelChunk* create(int x, int z);
    virtual LevelChunk* getChunk(int x, int z);
    virtual LevelChunk* getChunkLoadedOrUnloaded(int x, int z);
    void saveEntities(LevelChunk* chunk);
    void save(LevelChunk* chunk);
    virtual void flagPostProcessComplete(short index, int x, int z);
    virtual void saveAllEntities();
    virtual void save(bool force, ProgressListener* listener);
    virtual void tick();
    virtual bool shouldSave();
    virtual std::wstring gatherStats();
    virtual std::vector<Biome::MobSpawnerData*> getMobsAt(MobCategory* category, const BlockPos& pos);
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos, bool takeBlock);
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int x, int z);
    virtual LevelChunk* getChunkAt(const BlockPos& pos);
    static unsigned int runSaveThreadProc(void* data);
    virtual int getLoadedChunks();
    void setOverrideBoundFunction(bool (*function)(void*, int, int), void* data);
    virtual LevelChunk** getCache();

private:
    OSFastMutex m_mutex;
    ServerLevel* m_level;
    ChunkStorage* m_storage;
    ChunkGenerator* m_generator;
    std::vector<LevelChunk*> m_chunks;
    std::vector<LevelChunk*> m_unloadedChunks;
    LevelChunk* m_emptyChunk;
    std::vector<LevelChunk*> m_loadedChunks;
    std::deque<LevelChunk*> m_toSave;
    int m_xCenter;
    int m_zCenter;
    int m_width;
    int m_halfWidth;
    bool m_saveAll;
    bool (*m_boundFunction)(void*, int, int);
    void* m_boundData;
};

#endif
