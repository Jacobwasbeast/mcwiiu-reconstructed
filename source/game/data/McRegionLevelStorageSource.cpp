#include "game/data/McRegionLevelStorageSource.hpp"

#include "game/core/File.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/McRegionLevelStorage.hpp"
#include "game/network/ProgressListener.hpp"
#include "game/util/Math.hpp"
#include "game/world/LevelData.hpp"

McRegionLevelStorageSource::McRegionLevelStorageSource(File file, DataFixerUpper* fixer)
    : DirectoryLevelStorageSource(file, fixer)
{
}

McRegionLevelStorageSource::~McRegionLevelStorageSource()
{
}

std::wstring McRegionLevelStorageSource::getName()
{
    return std::wstring(L"Scaevolus' McRegion");
}

std::vector<LevelSummary*>* McRegionLevelStorageSource::getLevelList()
{
    return new std::vector<LevelSummary*>();
}

void McRegionLevelStorageSource::clearAll()
{
}

boost::shared_ptr<LevelStorage> McRegionLevelStorageSource::selectLevel(
    ConsoleSaveFile* saveFile, const std::wstring& levelId, bool bCreateIfMissing)
{
    return boost::shared_ptr<LevelStorage>(new McRegionLevelStorage(
        saveFile, mBaseDir, levelId, bCreateIfMissing, mFixer));
}

bool McRegionLevelStorageSource::isConvertible(ConsoleSaveFile* saveFile,
                                               const std::wstring& levelId)
{
    LevelData* levelData = getDataTagFor(saveFile, levelId);
    if (levelData != 0)
    {
        if (levelData->getVersion() == 0)
        {
            delete levelData;
            return true;
        }
        delete levelData;
    }
    return false;
}

bool McRegionLevelStorageSource::requiresConversion(ConsoleSaveFile* saveFile,
                                                    const std::wstring& levelId)
{
    LevelData* levelData = getDataTagFor(saveFile, levelId);
    if (levelData != 0)
    {
        if (levelData->getVersion() == 0)
        {
            delete levelData;
            return true;
        }
        delete levelData;
    }
    return false;
}

bool McRegionLevelStorageSource::convertLevel(ConsoleSaveFile* saveFile,
                                              const std::wstring& levelId,
                                              ProgressListener* progressListener)
{
    return true;
}

void McRegionLevelStorageSource::eraseFolders(std::vector<File*>* folders, int current,
                                              int total, ProgressListener* progressListener)
{
    std::vector<File*>::iterator it;
    for (it = folders->begin(); it != folders->end(); ++it)
    {
        File* folder = *it;
        deleteRecursive(folder->listFiles());
        folder->_delete();
        ++current;
        progressListener->progressStagePercentage(Math::round(100.0 * current / total));
    }
}
