#include "game/data/McRegionLevelStorageSource.hpp"

#include "game/core/File.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/McRegionLevelStorage.hpp"
#include "game/network/ProgressListener.hpp"
#include "game/util/Math.hpp"

static void eraseFoldersRecursive(std::vector<File*>* files)
{
    std::vector<File*>::iterator it;
    for (it = files->begin(); it != files->end(); ++it)
    {
        File* file = *it;
        if (file->isDirectory())
        {
            eraseFoldersRecursive(file->listFiles());
        }
        file->_delete();
    }
}

McRegionLevelStorageSource::McRegionLevelStorageSource(File file, DataFixerUpper* fixer)
    : DirectoryLevelStorageSource(file, fixer), mFixer(fixer)
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
    File level(levelId);
    return boost::shared_ptr<LevelStorage>(new McRegionLevelStorage(
        saveFile, level, levelId, bCreateIfMissing, mFixer));
}

bool McRegionLevelStorageSource::isConvertible(ConsoleSaveFile* saveFile,
                                               const std::wstring& levelId)
{
    return saveFile != 0 && !levelId.empty() && saveFile->getSizeOnDisk() != 0;
}

bool McRegionLevelStorageSource::requiresConversion(ConsoleSaveFile* saveFile,
                                                    const std::wstring& levelId)
{
    return saveFile != 0 && !levelId.empty() && saveFile->getSizeOnDisk() != 0;
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
        eraseFoldersRecursive(folder->listFiles());
        folder->_delete();
        ++current;
        progressListener->progressStage(Math::round(100.0 * current / total));
    }
}
