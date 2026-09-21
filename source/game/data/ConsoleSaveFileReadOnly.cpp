#include "game/data/ConsoleSaveFileReadOnly.hpp"

#include <cstring>

#include "platform/wiiu/CStorage.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/world/LevelGenerationOptions.hpp"

#ifdef RECON_NATIVE
#include "win32_memory.hpp"
#else
void* VirtualAlloc(void* address, unsigned long size, unsigned int allocationType, unsigned int protect);
int VirtualFree(void* address, unsigned long size, unsigned int freeType);
#endif
void __debugbreak();

static void* sSaveData = NULL;
static unsigned int sSaveDataPages = 0;

ConsoleSaveFileReadOnly::ConsoleSaveFileReadOnly(const std::wstring& filename, void* data, unsigned int dataSize, bool force, ESavePlatform platform)
    : ConsoleSaveFileCommon()
{
    bool deleteBaseSaveData = false;
    unsigned int saveDataSize = dataSize;

    if (sSaveData == NULL)
        sSaveData = VirtualAlloc(NULL, 0x4000000, 0x2000, 4);
    mData = sSaveData;

    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (data == NULL)
    {
        if (options != NULL && options->requiresBaseSave() &&
            (data = options->getBaseSaveData(saveDataSize)) != NULL && saveDataSize != 0)
            deleteBaseSaveData = true;
    }

    if (saveDataSize == 0)
    {
        gStorage->GetSaveData(NULL, &saveDataSize);
    }

    if (force)
        saveDataSize = 0;

    _initSaveFromData(filename, saveDataSize, data, platform);

    if (deleteBaseSaveData)
        options->deleteBaseSaveData();
}

ConsoleSaveFileReadOnly::~ConsoleSaveFileReadOnly()
{
    VirtualFree(sSaveData, 0x4000000, 0x4000);
    sSaveDataPages = 0;
}

void ConsoleSaveFileReadOnly::refreshFromLevelgen()
{
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    unsigned int saveDataSize = 0;
    void* saveData;

    if (options == NULL || !options->requiresBaseSave())
        return;

    saveData = options->getBaseSaveData(saveDataSize);
    if (saveData == NULL || saveDataSize == 0)
        return;

    ESavePlatform platform = mFileHeader.m_savePlatform;
    VirtualFree(sSaveData, 0x4000000, 0x4000);
    sSaveDataPages = 0;
    mFileHeader.Reset();
    _initSaveFromData(mFilename, saveDataSize, saveData, platform);
    options->deleteBaseSaveData();
}

void ConsoleSaveFileReadOnly::AllocInitialQuota(unsigned int size)
{
    if (size < 0x200000)
        size = 0x200000;

    VirtualAlloc(sSaveData, (size + 0xffff) & 0xffff0000, 0x1000, 4);
    sSaveDataPages = (size + 0xffff) >> 16;
}

void ConsoleSaveFileReadOnly::ReAlloc(unsigned int size)
{
    unsigned int pages = sSaveDataPages;

    if ((sSaveDataPages << 16) < size)
    {
        pages = (size + 0xffff) >> 16;
        if (VirtualAlloc(sSaveData, (size + 0xffff) & 0xffff0000, 0x1000, 4) == NULL)
            __debugbreak();
    }

    sSaveDataPages = pages;
}

FileEntry* ConsoleSaveFileReadOnly::createFile(const ConsoleSavePath& path)
{
    LockSaveAccess();
    FileEntry* entry = mFileHeader.AddFile(path.getPath(), 0);
    ReleaseSaveAccess();
    return entry;
}

void ConsoleSaveFileReadOnly::deleteFile(FileEntry* entry)
{
    _deleteFile(entry);
}

void ConsoleSaveFileReadOnly::setFilePointer(FileEntry* entry, long distanceToMove, int*, unsigned int moveMethod)
{
    LockSaveAccess();
    long filePointer = static_cast<long>(entry->m_filePointer) + distanceToMove;
    if (moveMethod == 2)
        filePointer += entry->m_fileSize;
    entry->m_filePointer = static_cast<unsigned int>(filePointer);
    ReleaseSaveAccess();
}

bool ConsoleSaveFileReadOnly::writeFile(FileEntry* entry, const void* buffer, unsigned int bytesToWrite, unsigned int* bytesWritten)
{
    return _writeFile(entry, buffer, bytesToWrite, bytesWritten);
}

bool ConsoleSaveFileReadOnly::zeroFile(FileEntry* entry, unsigned int bytesToZero, unsigned int* bytesWritten)
{
    return _zeroFile(entry, bytesToZero, bytesWritten);
}

bool ConsoleSaveFileReadOnly::readFile(FileEntry* entry, void* buffer, unsigned int bytesToRead, unsigned int* bytesRead)
{
    return _readFile(entry, buffer, bytesToRead, bytesRead);
}

void ConsoleSaveFileReadOnly::Flush(bool commit, bool force)
{
    (void)commit;
    (void)force;
}

std::vector<FileEntry*>* ConsoleSaveFileReadOnly::getRegionFilesByDimension(unsigned int dimension)
{
    (void)dimension;
    std::vector<FileEntry*>* regionFiles = NULL;
    return regionFiles;
}
