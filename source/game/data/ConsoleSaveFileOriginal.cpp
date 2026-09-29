#include "platform/wiiu/PlatformTiming.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/data/ConsoleSaveFileOriginal.hpp"

#include <cstring>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/Compression.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/FileHeader.hpp"
#include "game/world/LevelGenerationOptions.hpp"

#ifdef RECON_NATIVE
#include "win32_memory.hpp"
#include "game/core/PIXNamedEvents.hpp"
#else
void* VirtualAlloc(void* address, unsigned long size, unsigned int allocationType, unsigned int protect);
int VirtualFree(void* address, unsigned long size, unsigned int freeType);
#endif
#if defined(__POWERPC__) || defined(__powerpc__) || defined(__PPC__)
void __debugbreak()
{
}
#endif

static void* s_saveData;
static unsigned int s_saveDataSize;
extern CStorage* gStorage;

ConsoleSaveFileOriginal::ConsoleSaveFileOriginal(const std::wstring& filename, void* data, unsigned int dataSize,
    bool resetData, ESavePlatform platform)
    : ConsoleSaveFileCommon()
{
    if (s_saveData == NULL)
        s_saveData = VirtualAlloc(NULL, 0x4000000, 0x2000, 4);
    mData = s_saveData;

    bool deleteBaseSaveData = false;
    unsigned int saveDataSize = dataSize;
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (data == NULL)
    {
        if (options != NULL && options->requiresBaseSave() &&
            (data = options->getBaseSaveData(saveDataSize)) != NULL && saveDataSize != 0)
            deleteBaseSaveData = true;
        else
            saveDataSize = gStorage->GetSaveSize();
    }
    else if (saveDataSize == 0)
    {
        saveDataSize = gStorage->GetSaveSize();
    }

    if (resetData)
        saveDataSize = 0;

    _initSaveFromData(filename, saveDataSize, data, platform);
    if (deleteBaseSaveData)
        options->deleteBaseSaveData();
}

ConsoleSaveFileOriginal::~ConsoleSaveFileOriginal()
{
    VirtualFree(s_saveData, 0x4000000, 0x4000);
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
    VirtualAlloc(s_saveData, (size + 0xffff) & 0xffff0000, 0x1000, 4);
    s_saveDataSize = (size + 0xffff) >> 16;
}

void ConsoleSaveFileOriginal::ReAlloc(unsigned int size)
{
    if ((s_saveDataSize << 16) < size)
    {
        s_saveDataSize = (size + 0xffff) >> 16;
        if (VirtualAlloc(s_saveData, (size + 0xffff) & 0xffff0000, 0x1000, 4) == NULL)
            __debugbreak();
    }
}

FileEntry* ConsoleSaveFileOriginal::createFile(const ConsoleSavePath& path)
{
    LockSaveAccess();
    FileEntry* entry = mFileHeader.AddFile(path.getPath(), 0);
    ReAlloc(mFileHeader.GetFileSize());
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
    unsigned char* compressedData = static_cast<unsigned char*>(
        gStorage->AllocateSaveData(compressedSize));
    Compression* compression = Compression::getCompression();

    if (compressedData == NULL)
    {
        PIXBeginNamedEvent(0.0f, "Pre-calc save compression");
        _LARGE_INTEGER frequency;
        _LARGE_INTEGER counter;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&counter);
        compressedSize = 0;
        compression->Compress(NULL, &compressedSize, mData, fileSize);
        QueryPerformanceCounter(&counter);
        PIXEndNamedEvent();
        compressedSize += 8;
        compressedData = static_cast<unsigned char*>(
            gStorage->AllocateSaveData(compressedSize));
    }

    if (compressedData != NULL)
    {
        PIXBeginNamedEvent(0.0f, "Actual save compression");
        _LARGE_INTEGER frequency;
        _LARGE_INTEGER counter;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&counter);
        compression->Compress(compressedData + 8, &compressedSize, mData, fileSize);
        QueryPerformanceCounter(&counter);
        PIXEndNamedEvent();

        unsigned int zero = 0;
        std::memset(compressedData, 0, 8);
        std::memcpy(compressedData, &zero, sizeof(zero));
        std::memcpy(compressedData + 4, &fileSize, sizeof(fileSize));

        unsigned char* image = NULL;
        unsigned int imageSize = 0;
        g_pApp->GetSaveThumbnail(&image, &imageSize);
        unsigned char thumbnail[0x200];
        unsigned int thumbnailSize = fjGetSaveThumbnailData(thumbnail, sizeof(thumbnail));
        int saveUniqueNumber = 0;
        gStorage->GetSaveUniqueNumber(&saveUniqueNumber);
        telemetryManagerInstance->RecordLevelSaveOrCheckpoint(gProfile->GetPrimaryPad(), saveUniqueNumber,
                                                              compressedSize + 8);
        gStorage->SetSaveImages(image, imageSize, NULL, 0, thumbnail, thumbnailSize);
        delete[] image;
        gStorage->SetSaveDataSize(compressedSize + 8);
        gStorage->SaveSaveData(SaveSaveDataCallback, this);
    }
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

