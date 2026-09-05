#include "game/data/DirectoryLevelStorage.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/NbtIo.hpp"
#include "game/core/System.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/ConsoleSaveFileInputStream.hpp"
#include "game/data/ConsoleSaveFileOutputStream.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"
#include "game/world/generation/StructureManager.hpp"

DirectoryLevelStorage::DirectoryLevelStorage(ConsoleSaveFile* saveFile, File folder,
                                             const std::wstring& levelId, bool createIfMissing,
                                             DataFixerUpper* fixer)
    : mSaveFile(saveFile), mFolder(folder), mLevelId(levelId), mPlayersPrefix(L"players/"),
      mDataPrefix(L"data_"), mCreateIfMissing(createIfMissing), mSessionLock(0), mFixer(fixer),
      mStructureManager(0)
{
    mSessionLock = System::processTimeInMilliSecs();
    if (mCreateIfMissing)
        mStructureManager = new StructureManager(mFixer);
}

DirectoryLevelStorage::~DirectoryLevelStorage()
{
    saveAllCachedData();
    delete mStructureManager;
    mStructureManager = 0;
}

void DirectoryLevelStorage::dontSaveMapMappingForPlayer(PlayerUID uid)
{
    boost::unordered_map<PlayerUID, PlayerMappings, boost::hash<PlayerUID>, std::equal_to<PlayerUID> >::iterator it = mPlayerMappings.find(uid);
    if (it != mPlayerMappings.end())
        mPlayerMappings.erase(it);
}

void DirectoryLevelStorage::save(boost::shared_ptr<Player> player)
{
    if (!player)
        return;
}

CompoundTag* DirectoryLevelStorage::loadPlayerDataTag(PlayerUID uid)
{
    std::wstring path = mPlayersPrefix + uid.toString() + L"_dat";
    boost::unordered_map<std::wstring, ByteArrayOutputStream*>::iterator it = mCachedData.find(path);
    if (it != mCachedData.end() && it->second != 0)
    {
        ByteArrayInputStream input(it->second->toByteArray());
        return NbtIo::readCompressed(&input);
    }
    ConsoleSavePath savePath(path);
    if (!mSaveFile || !mSaveFile->doesFileExist(savePath))
        return 0;
    ConsoleSaveFileInputStream input(mSaveFile, savePath);
    return NbtIo::readCompressed(&input);
}

CompoundTag* DirectoryLevelStorage::load(boost::shared_ptr<Player> player)
{
    if (!player)
        return 0;
    return 0;
}

void DirectoryLevelStorage::deleteMapFilesForPlayer(PlayerUID uid)
{
    dontSaveMapMappingForPlayer(uid);
}

void DirectoryLevelStorage::clearOldPlayerFiles()
{
}

void DirectoryLevelStorage::saveMapIdLookup()
{
    if (!mSaveFile)
        return;
    ConsoleSavePath path(L"map_id_lookup");
    ConsoleSaveFileOutputStream output(mSaveFile, path);
    DataOutputStream data(&output);
    data.writeInt((int)mPlayerMappings.size());
    boost::unordered_map<PlayerUID, PlayerMappings, boost::hash<PlayerUID>, std::equal_to<PlayerUID> >::iterator it;
    for (it = mPlayerMappings.begin(); it != mPlayerMappings.end(); ++it)
    {
        data.writePlayerUID(it->first);
        it->second.writeMappings(&data);
    }
}

void DirectoryLevelStorage::deleteMapFilesForPlayer(boost::shared_ptr<Player> player)
{
    if (!player)
        return;
}

void DirectoryLevelStorage::saveAllCachedData()
{
    boost::unordered_map<std::wstring, ByteArrayOutputStream*>::iterator it;
    for (it = mCachedData.begin(); it != mCachedData.end(); ++it)
    {
        if (it->second == 0 || !mSaveFile)
            continue;
        ConsoleSaveFileOutputStream output(mSaveFile, ConsoleSavePath(it->first));
        output.write(it->second->toByteArray());
    }
}

void DirectoryLevelStorage::flushSaveFile(bool force)
{
    if (mSaveFile)
        mSaveFile->Flush(force, true);
}

LevelData* DirectoryLevelStorage::prepareLevel()
{
    ConsoleSavePath path(L"level.dat");
    if (!mSaveFile || !mSaveFile->doesFileExist(path))
        return 0;
    ConsoleSaveFileInputStream input(mSaveFile, path);
    CompoundTag* root = NbtIo::readCompressed(&input);
    if (!root)
        return 0;
    CompoundTag* data = root->getCompound(L"Data");
    if (mFixer)
        data = mFixer->updateData((DataFixTypes::EDataFixTypes)0, data);
    LevelData* result = new LevelData(data);
    delete root;
    return result;
}

void DirectoryLevelStorage::checkSession()
{
}

OldChunkStorage* DirectoryLevelStorage::createChunkStorage(Dimension* dimension)
{
    return dimension == 0 ? 0 : dimension->createStorage(mFolder);
}

void DirectoryLevelStorage::saveLevelData(LevelData* levelData,
                                          std::vector<boost::shared_ptr<Player> >* players)
{
    if (!levelData || !mSaveFile)
        return;
    CompoundTag* tag = players == 0 ? levelData->createTag() : levelData->createTag(players);
    ConsoleSaveFileOutputStream output(mSaveFile, ConsoleSavePath(L"level.dat"));
    NbtIo::writeCompressed(tag, &output);
    delete tag;
}

void DirectoryLevelStorage::saveLevelData(LevelData* levelData)
{
    saveLevelData(levelData, 0);
}

PlayerIO* DirectoryLevelStorage::getPlayerIO()
{
    return this;
}

void DirectoryLevelStorage::closeAll()
{
    saveAllCachedData();
    flushSaveFile(true);
}

File DirectoryLevelStorage::getFolder()
{
    return mFolder;
}

ConsoleSavePath DirectoryLevelStorage::getDataFile(const std::wstring& name)
{
    return ConsoleSavePath(mDataPrefix + name);
}

std::wstring DirectoryLevelStorage::getLevelId()
{
    return mLevelId;
}

StructureManager* DirectoryLevelStorage::getStructureManager()
{
    return mStructureManager;
}

ConsoleSaveFile* DirectoryLevelStorage::getSaveFile()
{
    return mSaveFile;
}

int DirectoryLevelStorage::getAuxValueForMap(PlayerUID uid, int dimension, int x, int z, int scale)
{
    boost::unordered_map<PlayerUID, PlayerMappings, boost::hash<PlayerUID>, std::equal_to<PlayerUID> >::iterator it = mPlayerMappings.find(uid);
    if (it == mPlayerMappings.end())
        return -1;
    int mapId = -1;
    if (!it->second.getMapping(mapId, x, z, dimension, scale))
        return -1;
    return mapId;
}

void DirectoryLevelStorage::resetPlayerPositionsInDimension(int dimension)
{
    (void)dimension;
}

void DirectoryLevelStorage::resetNetherPlayerPositions()
{
    resetPlayerPositionsInDimension(-1);
}

void DirectoryLevelStorage::resetEndPlayerPositions()
{
    resetPlayerPositionsInDimension(1);
}

void DirectoryLevelStorage::PlayerMappings::writeMappings(DataOutputStream* output)
{
    if (!output)
        return;
    output->writeInt((int)mMappings.size());
    boost::unordered_map<long long, short>::iterator it;
    for (it = mMappings.begin(); it != mMappings.end(); ++it)
    {
        output->writeLong(it->first);
        output->writeShort(it->second);
    }
}

void DirectoryLevelStorage::PlayerMappings::addMapping(int mapId, int x, int z, int dimension, int scale)
{
    long long key = ((long long)(((z & 0x1fffffff) << 2) | ((x >> 27) & 3)) << 32) |
                    (unsigned int)((x << 5) | ((scale & 7) << 2) | (dimension & 3));
    mMappings[key] = (short)mapId;
}

void DirectoryLevelStorage::PlayerMappings::readMappings(DataInputStream* input)
{
    if (!input)
        return;
    int count = input->readInt();
    while (count-- > 0)
        mMappings[input->readLong()] = input->readShort();
}

bool DirectoryLevelStorage::PlayerMappings::getMapping(int& mapId, int x, int z, int dimension, int scale)
{
    long long key = ((long long)(((z & 0x1fffffff) << 2) | ((x >> 27) & 3)) << 32) |
                    (unsigned int)((x << 5) | ((scale & 7) << 2) | (dimension & 3));
    boost::unordered_map<long long, short>::iterator it = mMappings.find(key);
    if (it == mMappings.end())
        return false;
    mapId = it->second;
    return true;
}
