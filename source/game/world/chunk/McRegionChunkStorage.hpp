#ifndef GAME_WORLD_CHUNK_MCREGIONCHUNKSTORAGE_HPP
#define GAME_WORLD_CHUNK_MCREGIONCHUNKSTORAGE_HPP

#include <string>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/ChunkStorage.hpp"

class ConsoleSaveFile;
class DataFixerUpper;
class Level;
class LevelChunk;

class McRegionChunkStorage : public ChunkStorage
{
public:
    typedef boost::unordered_map<long long, arrayWithLength<unsigned char> > RegionDataMap;
    McRegionChunkStorage(ConsoleSaveFile* saveFile, const std::wstring& directory, DataFixerUpper* fixer);
    virtual ~McRegionChunkStorage();
    static void runSaveThreadProc(void* argument);
    static void staticCtor();
    virtual LevelChunk* load(Level* level, int x, int z, LevelChunk* chunk);
    virtual void save(Level* level, LevelChunk* chunk);
    virtual void saveEntities(Level* level, LevelChunk* chunk);
    virtual void loadEntities(Level* level, LevelChunk* chunk);
    virtual void tick();
    virtual void flush();
    static void addWalkers(DataFixerUpper* fixer);
    static void WaitForAllSaves();
    virtual void WaitForAll();
    static void WaitForSaves();
    virtual void WaitIfTooManyQueuedChunks();
private:
    DataFixerUpper* mFixer;
    std::wstring mDirectory;
    ConsoleSaveFile* mSaveFile;
    RegionDataMap mRegionData;
};

#endif
