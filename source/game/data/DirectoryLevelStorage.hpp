#ifndef GAME_DATA_DIRECTORYLEVELSTORAGE_HPP
#define GAME_DATA_DIRECTORYLEVELSTORAGE_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/File.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/world/generation/PlayerIO.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"

class ByteArrayOutputStream;
class CompoundTag;
class ConsoleSaveFile;
class DataFixerUpper;
class DataInputStream;
class DataOutputStream;
class Dimension;
class LevelData;
class Player;
class StructureManager;

class DirectoryLevelStorage : public LevelStorage, public PlayerIO
{
public:
    class PlayerMappings
    {
    public:
        void writeMappings(DataOutputStream* output);
        void addMapping(int mapId, int x, int z, int dimension, int scale);
        void readMappings(DataInputStream* input);
        bool getMapping(int& mapId, int x, int z, int dimension, int scale);

    private:
        boost::unordered_map<long long, short> mMappings;
    };

    DirectoryLevelStorage(ConsoleSaveFile* saveFile, File folder, const std::wstring& levelId,
                          bool createIfMissing, DataFixerUpper* fixer);
    virtual ~DirectoryLevelStorage();
    void initiateSession();

    void dontSaveMapMappingForPlayer(PlayerUID uid);
    virtual void save(boost::shared_ptr<Player> player);
    CompoundTag* loadPlayerDataTag(PlayerUID uid);
    virtual CompoundTag* load(boost::shared_ptr<Player> player);
    void deleteMapFilesForPlayer(PlayerUID uid);
    virtual void clearOldPlayerFiles();
    virtual void saveMapIdLookup();
    virtual void deleteMapFilesForPlayer(boost::shared_ptr<Player> player);
    virtual void saveAllCachedData();

    virtual void flushSaveFile(bool force);
    virtual LevelData* prepareLevel();
    virtual void checkSession();
    virtual OldChunkStorage* createChunkStorage(Dimension* dimension);
    virtual void saveLevelData(LevelData* levelData, std::vector<boost::shared_ptr<Player> >* players);
    virtual void saveLevelData(LevelData* levelData);
    virtual PlayerIO* getPlayerIO();
    virtual void closeAll();
    virtual File getFolder();
    virtual ConsoleSavePath getDataFile(const std::wstring& name);
    virtual std::wstring getLevelId();
    virtual StructureManager* getStructureManager();
    virtual ConsoleSaveFile* getSaveFile();
    virtual int getAuxValueForMap(PlayerUID uid, int dimension, int x, int z, int scale);
    void resetPlayerPositionsInDimension(int dimension);
    void resetNetherPlayerPositions();
    void resetEndPlayerPositions();

private:
    ConsoleSaveFile* mSaveFile;
    File mFolder;
    std::wstring mLevelId;
    std::wstring mPlayersPrefix;
    std::wstring mDataPrefix;
    bool mCreateIfMissing;
    long long mSessionLock;
    DataFixerUpper* mFixer;
    StructureManager* mStructureManager;
    boost::unordered_map<PlayerUID, PlayerMappings, boost::hash<PlayerUID>, std::equal_to<PlayerUID> > mPlayerMappings;
    boost::unordered_map<std::wstring, ByteArrayOutputStream*> mCachedData;
    std::vector<short> mMapIds;
};

#endif
