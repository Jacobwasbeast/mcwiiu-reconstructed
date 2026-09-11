#include "game/data/DirectoryLevelStorage.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/NbtIo.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/System.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DoubleTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/ConsoleSaveFileInputStream.hpp"
#include "game/data/ConsoleSaveFileOutputStream.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"

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
    PlayerUID uid = player->mXuid;
    if (uid == PlayerUID(0)) {
        dontSaveMapMappingForPlayer(uid);
        return;
    }
    CompoundTag* tag = new CompoundTag();
    player->saveWithoutId(tag);
    std::wstring path = mPlayersPrefix + uid.toString() + L"_dat";
    if (mSaveFile) {
        ConsoleSaveFileOutputStream output(mSaveFile, ConsoleSavePath(path));
        NbtIo::writeCompressed(tag, &output);
    } else {
        ByteArrayOutputStream* bytes = new ByteArrayOutputStream();
        NbtIo::writeCompressed(tag, bytes);
        boost::unordered_map<std::wstring, ByteArrayOutputStream*>::iterator old = mCachedData.find(path);
        if (old != mCachedData.end()) delete old->second;
        mCachedData[path] = bytes;
    }
    delete tag;
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
    CompoundTag* tag = loadPlayerDataTag(player->mXuid);
    if (tag != 0) {
        if (mFixer)
            tag = mFixer->updateData(static_cast<DataFixTypes::EDataFixTypes>(1), tag);
        player->load(tag);
    }
    return tag;
}

void DirectoryLevelStorage::deleteMapFilesForPlayer(PlayerUID uid)
{
    dontSaveMapMappingForPlayer(uid);
}

void DirectoryLevelStorage::clearOldPlayerFiles()
{
    if (!mSaveFile)
        return;
    std::vector<FileEntry*>* files = mSaveFile->getFilesWithPrefix(mPlayersPrefix);
    std::sort(files->begin(), files->end(), FileEntry::newestFirst);
    for (size_t i = 80; i < files->size(); ++i)
    {
        FileEntry* entry = files->at(i);
        if (entry == 0)
            continue;
        std::wstring id = entry->m_fileName;
        const std::wstring prefix = mPlayersPrefix;
        if (id.compare(0, prefix.size(), prefix) == 0)
            id.erase(0, prefix.size());
        const std::wstring suffix = L".dat";
        if (id.size() >= suffix.size() && id.compare(id.size() - suffix.size(), suffix.size(), suffix) == 0)
            id.erase(id.size() - suffix.size());
        PlayerUID uid(id);
        deleteMapFilesForPlayer(uid);
        mSaveFile->deleteFile(entry);
    }
    delete files;
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

ChunkStorage* DirectoryLevelStorage::createChunkStorage(Dimension* dimension)
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
    std::vector<FileEntry*>* files = mSaveFile->getFilesWithPrefix(mPlayersPrefix);
    if (!files)
        return;
    for (std::vector<FileEntry*>::iterator it = files->begin(); it != files->end(); ++it)
    {
        FileEntry* entry = *it;
        ConsoleSaveFileInputStream input(mSaveFile, entry);
        CompoundTag* tag = NbtIo::readCompressed(&input);
        if (!tag)
            continue;
        bool dimensionMatches = false;
        if (tag->contains(L"Dimension"))
        {
            if (tag->getInt(L"Dimension") == dimension)
                dimensionMatches = true;
        }
        if (dimensionMatches && tag->contains(L"Pos"))
        {
            ListTag* pos = tag->getList(L"Pos", 6);
            pos->set(1, new DoubleTag(1.7976931348623157e+308));
            ConsoleSaveFileOutputStream output(mSaveFile, entry);
            NbtIo::writeCompressed(tag, &output);
        }
        delete tag;
    }
    delete files;
}

void DirectoryLevelStorage::resetNetherPlayerPositions()
{
    if (gApp.GetResetNether() != 0)
        resetPlayerPositionsInDimension(-1);
}

void DirectoryLevelStorage::resetEndPlayerPositions()
{
    if (gApp.GetResetEnd() != 0)
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
