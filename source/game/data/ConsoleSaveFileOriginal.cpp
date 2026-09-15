#include "platform/wiiu/PlatformTiming.hpp"
#include "game/data/ConsoleSaveFileOriginal.hpp"

#include <cstring>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/Compression.hpp"
#include "game/core/FileHeader.hpp"
#include "game/world/LevelGenerationOptions.hpp"

void* VirtualAlloc(void* address, unsigned long size, unsigned int allocationType, unsigned int protect);
int VirtualFree(void* address, unsigned long size, unsigned int freeType);
#if defined(__POWERPC__) || defined(__powerpc__) || defined(__PPC__)
void __debugbreak()
{
}
#endif
void PIXBeginNamedEvent(float color, const char* format, ...);
void PIXEndNamedEvent();

static void* s_saveData;
static unsigned int s_saveDataSize;

ConsoleSaveFileOriginal::ConsoleSaveFileOriginal(const std::wstring& filename, void* data, unsigned int dataSize,
    bool resetData, ESavePlatform platform)
    : ConsoleSaveFileCommon()
{
    static unsigned int quota;
    static void* quotaBase;
    bool deleteBaseData = false;
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();

    if (quotaBase == NULL)
        quotaBase = VirtualAlloc(NULL, 0x4000000, 0x2000, 4);

    if (dataSize == 0 && options != NULL && options->requiresBaseSave())
    {
        data = options->getBaseSaveData(dataSize);
        if (data != NULL)
            deleteBaseData = true;
    }
    else if (dataSize != 0 && data == NULL)
        data = s_saveData;

    if (resetData)
        data = NULL;

    _initSaveFromData(filename, dataSize, data, platform);
    quota = dataSize;
    if (deleteBaseData)
        options->deleteBaseSaveData();
}

ConsoleSaveFileOriginal::~ConsoleSaveFileOriginal()
{
    VirtualFree(s_saveData, 0x4000000, 0x4000);
    s_saveData = NULL;
    s_saveDataSize = 0;
}

int ConsoleSaveFileOriginal::SaveSaveDataCallback(void*, bool)
{
    return 0;
}

void ConsoleSaveFileOriginal::refreshFromLevelgen()
{
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (options != NULL && options->requiresBaseSave())
    {
        unsigned int dataSize = 0;
        void* data = options->getBaseSaveData(dataSize);
        if (data != NULL)
        {
            VirtualFree(s_saveData, 0x4000000, 0x4000);
            s_saveData = NULL;
            s_saveDataSize = 0;
            mFileHeader.Reset();
            _initSaveFromData(mFilename, dataSize, data, mFileHeader.m_savePlatform);
            options->deleteBaseSaveData();
        }
    }
}

void ConsoleSaveFileOriginal::AllocInitialQuota(unsigned int size)
{
    if (size < 0x200000)
        size = 0x200000;
    s_saveData = VirtualAlloc(s_saveData, (size + 0xffff) & 0xffff0000, 0x1000, 4);
    s_saveDataSize = (size + 0xffff) >> 16;
    mData = s_saveData;
}

void ConsoleSaveFileOriginal::ReAlloc(unsigned int size)
{
    if ((s_saveDataSize << 16) < size)
    {
        s_saveDataSize = (size + 0xffff) >> 16;
        if (VirtualAlloc(s_saveData, (size + 0xffff) & 0xffff0000, 0x1000, 4) == NULL)
            __debugbreak();
    }
    mData = s_saveData;
}

FileEntry* ConsoleSaveFileOriginal::createFile(const ConsoleSavePath& path)
{
    LockSaveAccess();
    FileEntry* entry = mFileHeader.AddFile(path.getPath(), 0);
    AllocInitialQuota(mFileHeader.GetFileSize());
    finalizeWrite();
    ReleaseSaveAccess();
    return entry;
}

void ConsoleSaveFileOriginal::deleteFile(FileEntry* entry)
{
    _deleteFile(entry);
}

void ConsoleSaveFileOriginal::setFilePointer(FileEntry* entry, long distanceToMove, int*, unsigned int moveMethod)
{
    LockSaveAccess();
    entry->m_filePointer = entry->m_startOffset + distanceToMove;
    if (moveMethod == 2)
        entry->m_filePointer += entry->m_fileSize;
    ReleaseSaveAccess();
}

bool ConsoleSaveFileOriginal::writeFile(FileEntry* entry, const void* buffer, unsigned int bytesToWrite,
    unsigned int* bytesWritten)
{
    return _writeFile(entry, buffer, bytesToWrite, bytesWritten);
}

bool ConsoleSaveFileOriginal::zeroFile(FileEntry* entry, unsigned int bytesToZero, unsigned int* bytesWritten)
{
    return _zeroFile(entry, bytesToZero, bytesWritten);
}

bool ConsoleSaveFileOriginal::readFile(FileEntry* entry, void* buffer, unsigned int bytesToRead, unsigned int* bytesRead)
{
    return _readFile(entry, buffer, bytesToRead, bytesRead);
}

void ConsoleSaveFileOriginal::Flush(bool, bool)
{
    LockSaveAccess();
    finalizeWrite();
    unsigned int fileSize = mFileHeader.GetFileSize();
    unsigned int compressedSize = fileSize + 8;
    unsigned char* compressedData = new unsigned char[compressedSize];
    Compression* compression = Compression::getCompression();
    compression->Compress(compressedData + 8, &compressedSize, mData, fileSize);
    std::memset(compressedData, 0, 8);
    unsigned char thumbnail[0x200];
    g_pApp->GetSaveThumbnail(NULL, NULL);
    fjGetSaveThumbnailData(thumbnail, sizeof(thumbnail));
    delete[] compressedData;
    ReleaseSaveAccess();
}

std::vector<FileEntry*>* ConsoleSaveFileOriginal::getRegionFilesByDimension(unsigned int)
{
    return NULL;
}

unsigned char* ConsoleSaveFileOriginal::getWritePointer(FileEntry* entry)
{
    return static_cast<unsigned char*>(mData) + entry->m_filePointer;
}

