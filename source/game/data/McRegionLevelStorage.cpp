#include "game/data/McRegionLevelStorage.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/RegionFileCache.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/NetherDimension.hpp"
#include "game/world/TheEndDimension.hpp"
#include "game/world/chunk/McRegionChunkStorage.hpp"
#include "game/core/MemSect.hpp"

static void clearFiles(ConsoleSaveFile* saveFile, const std::wstring& prefix)
{
    std::vector<FileEntry*>* files = saveFile->getFilesWithPrefix(prefix);
    if (files != NULL)
    {
        std::vector<FileEntry*>::iterator it;
        for (it = files->begin(); it != files->end(); ++it)
            saveFile->deleteFile(*it);
        delete files;
    }
}

McRegionLevelStorage::McRegionLevelStorage(ConsoleSaveFile* saveFile, File folder,
                                           const std::wstring& levelId, bool createIfMissing,
                                           DataFixerUpper* fixer)
    : DirectoryLevelStorage(saveFile, folder, levelId, createIfMissing, fixer)
{
    gRegionFileCache._clear();
}

McRegionLevelStorage::~McRegionLevelStorage()
{
    gRegionFileCache._clear();
}

McRegionChunkStorage* McRegionLevelStorage::createChunkStorage(Dimension* dimension)
{
    if (dimension != NULL && dynamic_cast<NetherDimension*>(dimension) != NULL)
    {
        if (gApp.GetResetNether() == 0)
            return new McRegionChunkStorage(getSaveFile(), L"DIM-1", mFixer);

        clearFiles(getSaveFile(), L"DIM-1");
        clearFiles(getSaveFile(), L"data/Fortress");
        resetNetherPlayerPositions();
        return new McRegionChunkStorage(getSaveFile(), L"DIM-1", mFixer);
    }

    if (dimension != NULL && dynamic_cast<TheEndDimension*>(dimension) != NULL)
    {
        if (gApp.GetResetEnd() == 0)
        {
            short saveVersion = getSaveFile()->getSaveVersion();
            if (saveVersion == 0 || saveVersion > 3)
                return new McRegionChunkStorage(getSaveFile(), L"DIM1/", mFixer);
        }

        clearFiles(getSaveFile(), L"DIM1/");
        resetEndPlayerPositions();
        return new McRegionChunkStorage(getSaveFile(), L"DIM1/", mFixer);
    }

    return new McRegionChunkStorage(getSaveFile(), L"", mFixer);
}

void McRegionLevelStorage::saveLevelData(LevelData* levelData,
                                         std::vector<boost::shared_ptr<Player> >* players)
{
    levelData->setVersion(19132);
    MemSect(38);
    DirectoryLevelStorage::saveLevelData(levelData, players);
    MemSect(0);
}

void McRegionLevelStorage::closeAll()
{
    gRegionFileCache._clear();
}
