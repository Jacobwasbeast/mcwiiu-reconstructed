#include "game/data/DirectoryLevelStorage.hpp"

#include <algorithm>
#include <cfloat>

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/FileEntry.hpp"
#include "game/core/NbtIo.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/core/System.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/ConsoleSaveFileInputStream.hpp"
#include "game/data/ConsoleSaveFileOutputStream.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/DirectoryLevelStorageSource.hpp"
#include "game/data/DoubleTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/NetherDimension.hpp"
#include "game/world/TheEndDimension.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "vendor/_MapDataMappings.hpp"
#include "vendor/_MapDataMappings_old.hpp"

static const std::wstring PLAYER_DIR = L"players/";
static const std::wstring NETHER_FOLDER = L"DIM-1";
static const std::wstring ENDER_FOLDER = L"DIM1/";

DirectoryLevelStorage::DirectoryLevelStorage(ConsoleSaveFile* saveFile, File folder,
                                             const std::wstring& levelId, bool createIfMissing,
                                             DataFixerUpper* fixer)
    : mFolder(L""),
      mPlayerDir(PLAYER_DIR),
      mDataDir(L"data/"),
      mSessionId(System::processTimeInMilliSecs()),
      mLevelId(levelId),
      mHasLoadedMapDataMappings(false)
{
    mFixer = fixer;
    mSaveFile = saveFile;
    mUsedMappings = arrayWithLength<unsigned char>(32, true);
    mStructureManager = createIfMissing ? new StructureManager(fixer) : NULL;
}

DirectoryLevelStorage::~DirectoryLevelStorage()
{
    delete mSaveFile;

    for (CachedSaveDataMap::iterator it = mCachedSaveData.begin(); it != mCachedSaveData.end(); ++it)
        delete it->second;

    if (mStructureManager != NULL)
    {
        delete mStructureManager;
        mStructureManager = NULL;
    }
}

void DirectoryLevelStorage::dontSaveMapMappingForPlayer(PlayerUID uid)
{
    PlayerMappingsMap::iterator it = mPlayerMappings.find(uid);
    if (it == mPlayerMappings.end())
        return;

    boost::unordered_map<long long, short>& mappings = it->second.mMappings;
    for (boost::unordered_map<long long, short>::iterator mapping = mappings.begin(); mapping != mappings.end(); ++mapping)
    {
        short id = mapping->second;
        mUsedMappings.data[id / 8] &= ~(1 << (id % 8));
    }
    mPlayerMappings.erase(it);
}

void DirectoryLevelStorage::save(boost::shared_ptr<Player> player)
{
    PlayerUID uid = player->mXuid;
    if (uid != PlayerUID(0) && !player->mIsGuest)
    {
        CompoundTag* tag = new CompoundTag();
        player->saveWithoutId(tag);

        ConsoleSavePath file(mPlayerDir + player->mXuid.toString() + L".dat");
        if (!gStorage->GetSaveDisabled())
        {
            ConsoleSaveFileOutputStream output(mSaveFile, file);
            NbtIo::writeCompressed(tag, &output);
        }
        else
        {
            ByteArrayOutputStream* bytes = new ByteArrayOutputStream();
            NbtIo::writeCompressed(tag, bytes);

            CachedSaveDataMap::iterator it = mCachedSaveData.find(file.getPath());
            if (it != mCachedSaveData.end() && it->second != NULL)
                delete it->second;
            mCachedSaveData[file.getPath()] = bytes;
        }

        delete tag;
    }
    else if (uid != PlayerUID(0))
    {
        dontSaveMapMappingForPlayer(uid);
    }
}

CompoundTag* DirectoryLevelStorage::loadPlayerDataTag(PlayerUID uid)
{
    ConsoleSavePath file(mPlayerDir + uid.toString() + L".dat");

    CachedSaveDataMap::iterator it = mCachedSaveData.find(file.getPath());
    if (it != mCachedSaveData.end())
    {
        ByteArrayOutputStream* bytes = it->second;
        ByteArrayInputStream input(bytes->mBuffer, 0, bytes->mCount);
        CompoundTag* tag = NbtIo::readCompressed(&input);
        input.reset();
        return tag;
    }

    if (mSaveFile->doesFileExist(file))
    {
        ConsoleSaveFileInputStream input(mSaveFile, file);
        return NbtIo::readCompressed(&input);
    }
    return NULL;
}

CompoundTag* DirectoryLevelStorage::load(boost::shared_ptr<Player> player)
{
    CompoundTag* tag = loadPlayerDataTag(player->mXuid);
    if (tag != NULL)
        player->load(mFixer->updateData(DataFixTypes::eDataFixTypes_Player, tag));
    return tag;
}

void DirectoryLevelStorage::deleteMapFilesForPlayer(PlayerUID uid)
{
    PlayerMappingsMap::iterator it = mPlayerMappings.find(uid);
    if (it == mPlayerMappings.end())
        return;

    boost::unordered_map<long long, short>& mappings = it->second.mMappings;
    for (boost::unordered_map<long long, short>::iterator mapping = mappings.begin(); mapping != mappings.end(); ++mapping)
    {
        std::wstring prefix = L"map_";
        ConsoleSavePath file = getDataFile(prefix + _toString<short>(mapping->second));
        if (mSaveFile->doesFileExist(file))
        {
            if (gStorage->GetSaveDisabled())
                mMapFilesToDelete.push_back(mapping->second);
            else
                mSaveFile->deleteFile(mSaveFile->createFile(file));
        }

        short id = mapping->second;
        mUsedMappings.data[id / 8] &= ~(1 << (id % 8));
    }
    mPlayerMappings.erase(it);
}

void DirectoryLevelStorage::clearOldPlayerFiles()
{
    if (gStorage->GetSaveDisabled())
        return;

    std::vector<FileEntry*>* files = mSaveFile->getFilesWithPrefix(mPlayerDir);
    if (files == NULL)
        return;

    if (files->size() > MAX_PLAYER_DATA_SAVES)
    {
        std::sort(files->begin(), files->end(), FileEntry::newestFirst);

        for (unsigned int i = MAX_PLAYER_DATA_SAVES; i < files->size(); ++i)
        {
            FileEntry* file = files->at(i);
            std::wstring xuidString = replaceAll(replaceAll(file->m_fileName, mPlayerDir, L""), L".dat", L"");
            PlayerUID xuid(xuidString);
            deleteMapFilesForPlayer(xuid);
            mSaveFile->deleteFile(files->at(i));
        }
    }
    delete files;
}

void DirectoryLevelStorage::saveMapIdLookup()
{
    if (gStorage->GetSaveDisabled())
        return;

    ConsoleSavePath file = getDataFile(L"largeMapDataMappings");
    if (file.getPath().empty())
        return;

    FileEntry* entry = mSaveFile->createFile(file);
    mSaveFile->setFilePointer(entry, 0, NULL, 0);

    ByteArrayOutputStream bytes;
    DataOutputStream output(&bytes);
    output.writeInt(mPlayerMappings.size());
    for (PlayerMappingsMap::iterator it = mPlayerMappings.begin(); it != mPlayerMappings.end(); ++it)
    {
        output.writePlayerUID(it->first);
        it->second.writeMappings(&output);
    }
    output.write(mUsedMappings);

    unsigned int bytesWritten;
    mSaveFile->writeFile(entry, bytes.mBuffer.data, bytes.mCount, &bytesWritten);
}

void DirectoryLevelStorage::deleteMapFilesForPlayer(boost::shared_ptr<Player> player)
{
    PlayerUID uid = player->mXuid;
    if (uid != PlayerUID(0))
        deleteMapFilesForPlayer(uid);
}

void DirectoryLevelStorage::saveAllCachedData()
{
    if (gStorage->GetSaveDisabled())
        return;

    for (CachedSaveDataMap::iterator it = mCachedSaveData.begin(); it != mCachedSaveData.end(); ++it)
    {
        ByteArrayOutputStream* bytes = it->second;
        ConsoleSaveFileOutputStream output(mSaveFile, ConsoleSavePath(it->first));
        output.write(bytes->mBuffer, 0, bytes->mCount);
        delete bytes;
    }
    mCachedSaveData.clear();

    for (std::vector<short>::iterator it = mMapFilesToDelete.begin(); it != mMapFilesToDelete.end(); ++it)
    {
        std::wstring prefix = L"map_";
        ConsoleSavePath file = getDataFile(prefix + _toString<short>(*it));
        if (mSaveFile->doesFileExist(file))
            mSaveFile->deleteFile(mSaveFile->createFile(file));
    }
    mMapFilesToDelete.clear();
}

void DirectoryLevelStorage::flushSaveFile(bool force)
{
    mSaveFile->Flush(force, true);
}

LevelData* DirectoryLevelStorage::prepareLevel()
{
    ConsoleSavePath mapFile = getDataFile(L"largeMapDataMappings");
    bool legacyMappings = false;
    if (!mHasLoadedMapDataMappings && !getSaveFile()->doesFileExist(mapFile))
    {
        mapFile = getDataFile(L"mapDataMappings");
        legacyMappings = true;
    }

    if (!mHasLoadedMapDataMappings && getSaveFile()->doesFileExist(mapFile))
    {
        FileEntry* entry = getSaveFile()->createFile(mapFile);
        getSaveFile()->setFilePointer(entry, 0, NULL, 0);

        unsigned int bytesRead;
        if (legacyMappings)
        {
            if (getSaveFile()->getSaveVersion() < 5)
            {
                _MapDataMappings_old oldMapDataMappings;
                getSaveFile()->readFile(entry, &oldMapDataMappings, sizeof(oldMapDataMappings), &bytesRead);
                for (unsigned int id = 0; id < 256; ++id)
                {
                    if (oldMapDataMappings.xuids[id] != PlayerUID(0))
                    {
                        mPlayerMappings[oldMapDataMappings.xuids[id]].addMapping(id, 0, 0, oldMapDataMappings.getDimension(id), 3);
                        mUsedMappings.data[id >> 3] |= 1 << (id & 7);
                    }
                }
            }
            else
            {
                _MapDataMappings mapDataMappings;
                getSaveFile()->readFile(entry, &mapDataMappings, sizeof(mapDataMappings), &bytesRead);
                for (unsigned int id = 0; id < 256; ++id)
                {
                    if (mapDataMappings.xuids[id] != PlayerUID(0))
                    {
                        mPlayerMappings[mapDataMappings.xuids[id]].addMapping(id, 0, 0, mapDataMappings.getDimension(id), 3);
                        mUsedMappings.data[id >> 3] |= 1 << (id & 7);
                    }
                }
            }
        }
        else
        {
            arrayWithLength<unsigned char> data(entry->m_fileSize, true);
            getSaveFile()->readFile(entry, data.data, entry->m_fileSize, &bytesRead);

            ByteArrayInputStream bytes(data);
            DataInputStream input(&bytes);
            int count = input.readInt();
            for (int i = 0; i < count; ++i)
            {
                PlayerUID uid = input.readPlayerUID();
                mPlayerMappings[uid].readMappings(&input);
            }
            input.readFully(mUsedMappings);
        }

        if (getSaveFile()->getSaveVersion() < 5)
            saveMapIdLookup();

        mHasLoadedMapDataMappings = true;
    }

    std::wstring levelFileName = L"level.dat";
    ConsoleSavePath levelFile(levelFileName);
    if (mSaveFile->doesFileExist(levelFile))
    {
        LevelData* levelData = DirectoryLevelStorageSource::getLevelData(mSaveFile, levelFile, mFixer);
        if (levelData != NULL)
            return levelData;
    }
    return NULL;
}

void DirectoryLevelStorage::checkSession()
{
}

ChunkStorage* DirectoryLevelStorage::createChunkStorage(Dimension* dimension)
{
    if (dimension != NULL)
    {
        if (dynamic_cast<NetherDimension*>(dimension) != NULL)
            return new OldChunkStorage(File(mFolder, NETHER_FOLDER), true);

        if (dynamic_cast<TheEndDimension*>(dimension) != NULL)
            return new OldChunkStorage(File(mFolder, ENDER_FOLDER), true);
    }
    return new OldChunkStorage(mFolder, true);
}

void DirectoryLevelStorage::saveLevelData(LevelData* levelData,
                                          std::vector<boost::shared_ptr<Player> >* players)
{
    CompoundTag* dataTag = levelData->createTag(players);
    CompoundTag* root = new CompoundTag();
    root->put(L"Data", dataTag);

    ConsoleSaveFileOutputStream output(mSaveFile, ConsoleSavePath(L"level.dat"));
    NbtIo::writeCompressed(root, &output);
    delete root;
}

void DirectoryLevelStorage::saveLevelData(LevelData* levelData)
{
    CompoundTag* dataTag = levelData->createTag();
    CompoundTag* root = new CompoundTag();
    root->put(L"Data", dataTag);

    ConsoleSaveFileOutputStream output(mSaveFile, ConsoleSavePath(L"level.dat"));
    NbtIo::writeCompressed(root, &output);
    delete root;
}

PlayerIO* DirectoryLevelStorage::getPlayerIO()
{
    return this;
}

void DirectoryLevelStorage::closeAll()
{
}

File DirectoryLevelStorage::getFolder()
{
    return mFolder;
}

ConsoleSavePath DirectoryLevelStorage::getDataFile(const std::wstring& name)
{
    return ConsoleSavePath(mDataDir + name + L".dat");
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

int DirectoryLevelStorage::getAuxValueForMap(PlayerUID uid, int dimension, int centreX, int centreZ, int scale)
{
    int id = -1;
    PlayerMappingsMap::iterator it = mPlayerMappings.find(uid);
    if (it == mPlayerMappings.end() || !it->second.getMapping(id, centreX, centreZ, dimension, scale))
    {
        for (unsigned int i = 0; i < mUsedMappings.length; ++i)
        {
            if (mUsedMappings.data[i] == 0xff)
                continue;

            int bit = 0;
            while (bit < 8 && (mUsedMappings.data[i] & (1 << bit)) != 0)
                ++bit;

            id = i * 8 + bit;
            mPlayerMappings[uid].addMapping(id, centreX, centreZ, dimension, scale);
            mUsedMappings.data[i] |= 1 << bit;
            break;
        }
    }
    return id;
}

void DirectoryLevelStorage::resetPlayerPositionsInDimension(int dimension)
{
    std::vector<FileEntry*>* files = mSaveFile->getFilesWithPrefix(mPlayerDir);
    if (files == NULL)
        return;

    for (std::vector<FileEntry*>::iterator it = files->begin(); it != files->end(); ++it)
    {
        FileEntry* file = *it;
        ConsoleSaveFileInputStream input(mSaveFile, file);
        CompoundTag* tag = NbtIo::readCompressed(&input);
        if (tag == NULL)
            continue;

        if (tag->contains(L"Dimension") && tag->getInt(L"Dimension") == dimension)
        {
            if (tag->contains(L"Pos"))
            {
                ListTag* pos = tag->getList(L"Pos", Tag::TAG_DOUBLE);
                static_cast<DoubleTag*>(pos->get(1))->mValue = DBL_MAX;

                ConsoleSaveFileOutputStream output(mSaveFile, file);
                NbtIo::writeCompressed(tag, &output);
            }
        }
        delete tag;
    }
    delete files;
}

void DirectoryLevelStorage::resetNetherPlayerPositions()
{
    if (gApp.GetResetNether())
        resetPlayerPositionsInDimension(-1);
}

void DirectoryLevelStorage::resetEndPlayerPositions()
{
    if (gApp.GetResetEnd())
        resetPlayerPositionsInDimension(1);
}

void DirectoryLevelStorage::PlayerMappings::writeMappings(DataOutputStream* output)
{
    output->writeInt(mMappings.size());
    for (boost::unordered_map<long long, short>::iterator it = mMappings.begin(); it != mMappings.end(); ++it)
    {
        output->writeLong(it->first);
        output->writeInt(it->second);
    }
}

void DirectoryLevelStorage::PlayerMappings::addMapping(int id, int centreX, int centreZ, int dimension, int scale)
{
    long long index = ((long long)(((centreZ & 0x1FFFFFFF) << 2) | ((centreX >> 27) & 3)) << 32) |
                      (unsigned int)((centreX << 5) | ((scale & 7) << 2) | (dimension & 3));
    mMappings[index] = id;
}

void DirectoryLevelStorage::PlayerMappings::readMappings(DataInputStream* input)
{
    int count = input->readInt();
    for (int i = 0; i < count; ++i)
    {
        long long index = input->readLong();
        int id = input->readInt();
        mMappings[index] = id;
    }
}

bool DirectoryLevelStorage::PlayerMappings::getMapping(int& id, int centreX, int centreZ, int dimension, int scale)
{
    long long index = ((long long)(((centreZ & 0x1FFFFFFF) << 2) | ((centreX >> 27) & 3)) << 32) |
                      (unsigned int)((centreX << 5) | ((scale & 7) << 2) | (dimension & 3));
    boost::unordered_map<long long, short>::iterator it = mMappings.find(index);
    if (it == mMappings.end())
        return false;

    id = it->second;
    return true;
}
