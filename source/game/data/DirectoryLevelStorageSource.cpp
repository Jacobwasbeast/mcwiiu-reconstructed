#include "game/data/DirectoryLevelStorageSource.hpp"

#include "game/core/NbtIo.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ConsoleSaveFileInputStream.hpp"
#include "game/data/ConsoleSaveFileOriginal.hpp"
#include "game/data/ConsoleSaveFileOutputStream.hpp"
#include "game/data/ConsoleSavePath.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/DirectoryLevelStorage.hpp"
#include "game/world/LevelData.hpp"

DirectoryLevelStorageSource::DirectoryLevelStorageSource(File file, DataFixerUpper* fixer)
    : mBaseDir(file), mFixer(fixer)
{
}

DirectoryLevelStorageSource::~DirectoryLevelStorageSource()
{
}

LevelData* DirectoryLevelStorageSource::getLevelData(ConsoleSaveFile* saveFile, const ConsoleSavePath& path, DataFixerUpper* fixer)
{
    ConsoleSaveFileInputStream input(saveFile, path);
    CompoundTag* root = NbtIo::readCompressed(&input);
    CompoundTag* data = root->getCompound(std::wstring(L"Data"));
    data = fixer->updateData((DataFixTypes::EDataFixTypes)0, data);
    LevelData* levelData = new LevelData(data);
    if (root != 0)
    {
        delete root;
    }
    return levelData;
}

std::wstring DirectoryLevelStorageSource::getName()
{
    return std::wstring(L"Old_Format");
}

std::vector<LevelSummary*>* DirectoryLevelStorageSource::getLevelList()
{
    return new std::vector<LevelSummary*>();
}

void DirectoryLevelStorageSource::clearAll()
{
}

LevelData* DirectoryLevelStorageSource::getDataTagFor(ConsoleSaveFile* saveFile, const std::wstring& levelId)
{
    ConsoleSavePath path(L"level.dat");
    if (saveFile->doesFileExist(path))
    {
        return getLevelData(saveFile, path, mFixer);
    }
    return 0;
}

void DirectoryLevelStorageSource::renameLevel(const std::wstring& levelId, const std::wstring& levelName)
{
    ConsoleSaveFileOriginal saveFile(levelId, 0, 0, false, eSavePlatform_WIIU);
    ConsoleSavePath path(L"level.dat");
    if (saveFile.doesFileExist(path))
    {
        ConsoleSaveFileInputStream input(&saveFile, path);
        CompoundTag* root = NbtIo::readCompressed(&input);
        CompoundTag* data = root->getCompound(std::wstring(L"Data"));
        data->putString(std::wstring(L"LevelName"), levelName);
        ConsoleSaveFileOutputStream output(&saveFile, path);
        NbtIo::writeCompressed(root, &output);
    }
}

bool DirectoryLevelStorageSource::isNewLevelIdAcceptable(const std::wstring& levelId)
{
    File level(mBaseDir, levelId);
    if (level.exists())
    {
        return false;
    }
    level.mkdir();
    return true;
}

void DirectoryLevelStorageSource::deleteRecursive(std::vector<File*>* files)
{
    std::vector<File*>::iterator it;
    for (it = files->begin(); it != files->end(); ++it)
    {
        File* file = *it;
        if (file->isDirectory())
        {
            deleteRecursive(file->listFiles());
        }
        file->_delete();
    }
}

void DirectoryLevelStorageSource::deleteLevel(const std::wstring& levelId)
{
    File level(mBaseDir, levelId);
    if (level.exists())
    {
        deleteRecursive(level.listFiles());
        level._delete();
    }
}

boost::shared_ptr<LevelStorage> DirectoryLevelStorageSource::selectLevel(ConsoleSaveFile* saveFile, const std::wstring& levelId, bool bCreateIfMissing)
{
    return boost::shared_ptr<LevelStorage>(new DirectoryLevelStorage(saveFile, mBaseDir, levelId, bCreateIfMissing, mFixer));
}

bool DirectoryLevelStorageSource::isConvertible(ConsoleSaveFile* saveFile, const std::wstring& levelId)
{
    return false;
}

bool DirectoryLevelStorageSource::requiresConversion(ConsoleSaveFile* saveFile, const std::wstring& levelId)
{
    return false;
}

bool DirectoryLevelStorageSource::convertLevel(ConsoleSaveFile* saveFile, const std::wstring& levelId, ProgressListener* progressListener)
{
    return false;
}
