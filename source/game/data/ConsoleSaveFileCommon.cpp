#include "game/data/ConsoleSaveFileCommon.hpp"

#include <cstring>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/Compression.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/File.hpp"
#include "game/core/RegionFile.hpp"
#include "game/core/RegionFileCache.hpp"
#include "game/core/System.hpp"
#include "platform/wiiu/CStorage.hpp"

void PIXBeginNamedEvent(float color, const char* format, ...);
void PIXEndNamedEvent();
void* XMemCpy(void* destination, const void* source, unsigned int size);

ConsoleSaveFileCommon::ConsoleSaveFileCommon()
    : mFileHeader(), mFilename(L"Growing file")
{
    InitializeCriticalSectionAndSpinCount(&mSaveAccessMutex, 0x1400);
}

ConsoleSaveFileCommon::~ConsoleSaveFileCommon()
{
    DeleteCriticalSection(&mSaveAccessMutex);
}

void ConsoleSaveFileCommon::_initSaveFromData(const std::wstring& filename, unsigned int dataSize, void* data, ESavePlatform platform)
{
    mFilename = filename;
    AllocInitialQuota(dataSize);
    if (dataSize == 0)
    {
        mFileHeader.WriteHeader(mData);
        return;
    }

    void* saveData = data;
    if (data == NULL)
    {
        gStorage->GetSaveData(mData, &dataSize);
        saveData = mData;
    }
    else
        std::memcpy(mData, data, dataSize);

    unsigned int* compressedHeader = static_cast<unsigned int*>(saveData);
    if (compressedHeader[0] == 0)
    {
        unsigned int uncompressedSize = compressedHeader[1];
        if (isLocalEndianDifferent(platform))
            System::ReverseULONG(&uncompressedSize);

        if (uncompressedSize == 0)
        {
            std::memset(saveData, 0, dataSize);
            mFileHeader.WriteHeader(saveData);
            mFileHeader.ReadHeader(mData, platform);
            return;
        }

        ReAlloc(uncompressedSize);
        unsigned char* uncompressedData = new unsigned char[uncompressedSize];
        if (platform == eSavePlatform_PSV)
        {
            Compression::VitaVirtualDecompress(uncompressedData, &uncompressedSize, compressedHeader + 2, dataSize - 8);
            std::memcpy(mData, uncompressedData, uncompressedSize);
        }
        else
        {
            Compression* compression = Compression::getCompression();
            compression->SetDecompressionType(platform);
            int result = compression->Decompress(uncompressedData, &uncompressedSize, compressedHeader + 2, dataSize - 8);
            compression->SetDecompressionType(eSavePlatform_WIIU);
            if (result == 0)
                std::memcpy(mData, uncompressedData, uncompressedSize);
            else
            {
                std::memset(mData, 0, dataSize);
                mFileHeader.WriteHeader(mData);
            }
        }
        delete[] uncompressedData;
    }

    mFileHeader.ReadHeader(mData, platform);
}

void ConsoleSaveFileCommon::_deleteFile(FileEntry* entry)
{
    if (entry == NULL)
        return;
    LockSaveAccess();
    unsigned char buffer[0x1000];
    unsigned int destination = entry->m_startOffset;
    unsigned int source = destination + entry->m_fileSize;
    unsigned int end = mFileHeader.GetStartOfNextData();
    while (source < end)
    {
        unsigned int count = end - source;
        if (count > sizeof(buffer))
            count = sizeof(buffer);
        std::memcpy(buffer, static_cast<unsigned char*>(mData) + source, count);
        std::memcpy(static_cast<unsigned char*>(mData) + destination, buffer, count);
        source += count;
        destination += count;
    }
    mFileHeader.RemoveFile(entry);
    finalizeWrite();
    ReleaseSaveAccess();
}

void ConsoleSaveFileCommon::MoveDataBeyond(FileEntry* entry, unsigned int size)
{
    unsigned int fileSize = mFileHeader.GetFileSize();
    ReAlloc(fileSize + size);
    unsigned char* dataStart = static_cast<unsigned char*>(mData) + entry->m_startOffset + entry->m_fileSize;
    unsigned char* source = static_cast<unsigned char*>(mData) + mFileHeader.GetStartOfNextData();

    if ((size & 0xfff) == 0)
    {
        if (size != 0)
        {
            while (dataStart < source)
            {
                unsigned int count = static_cast<unsigned int>(source - dataStart);
                if (count > 0x1000)
                    count = 0x1000;
                source -= count;
                XMemCpy(source + size, source, count);
            }
        }
    }
    else
    {
        static unsigned char moveBuffer[0x1000];
        static unsigned char previousBuffer[0x1000];
        unsigned char* destination = source + size;
        unsigned int previousCount = 0;
        unsigned int count;
        do
        {
            std::memcpy(previousBuffer, moveBuffer, previousCount);
            count = static_cast<unsigned int>(source - dataStart);
            if (count > 0x1000)
                count = 0x1000;
            source -= count;
            std::memcpy(moveBuffer, source, count);
            destination -= previousCount;
            std::memcpy(destination, previousBuffer, previousCount);
            previousCount = count;
        }
        while (count != 0);
    }
    mFileHeader.AdjustStartOffsets(entry, size, false);
}

void ConsoleSaveFileCommon::PrepareForWrite(FileEntry* entry, unsigned int size)
{
    int delta = static_cast<int>(entry->m_filePointer - entry->m_startOffset + size - entry->m_fileSize);
    if (delta > 0)
    {
        PIXBeginNamedEvent(0.0f, "Growing file by %d bytes", delta);
        MoveDataBeyond(entry, static_cast<unsigned int>(delta));
        PIXEndNamedEvent();
        entry->m_fileSize += delta;
        finalizeWrite();
    }
}

bool ConsoleSaveFileCommon::_writeFile(FileEntry* entry, const void* buffer, unsigned int size, unsigned int* written)
{
    if (mData == NULL || entry == NULL)
        return false;
    LockSaveAccess();
    PrepareForWrite(entry, size);
    if (size != 0)
    {
        if (buffer == NULL)
        {
            ReleaseSaveAccess();
            return false;
        }
        std::memcpy(static_cast<unsigned char*>(mData) + entry->m_filePointer, buffer, size);
    }
    if (written != NULL)
        *written = size;
    entry->m_filePointer += size;
    entry->m_timestamp = System::currentRealTimeMillis();
    ReleaseSaveAccess();
    return true;
}

bool ConsoleSaveFileCommon::_zeroFile(FileEntry* entry, unsigned int size, unsigned int* written)
{
    if (mData == NULL)
        return false;
    LockSaveAccess();
    PrepareForWrite(entry, size);
    std::memset(static_cast<unsigned char*>(mData) + entry->m_filePointer, 0, size);
    *written = size;
    entry->m_filePointer += size;
    entry->m_timestamp = System::currentRealTimeMillis();
    ReleaseSaveAccess();
    return true;
}

bool ConsoleSaveFileCommon::_readFile(FileEntry* entry, void* buffer, unsigned int size, unsigned int* read)
{
    if (mData == NULL)
        return false;
    LockSaveAccess();
    unsigned int end = entry->m_startOffset + entry->m_fileSize;
    if (entry->m_filePointer + size > end)
        size = end - entry->m_filePointer;
    std::memcpy(buffer, static_cast<unsigned char*>(mData) + entry->m_filePointer, size);
    *read = size;
    entry->m_filePointer += size;
    ReleaseSaveAccess();
    return true;
}

bool ConsoleSaveFileCommon::closeHandle(FileEntry*)
{
    LockSaveAccess();
    finalizeWrite();
    ReleaseSaveAccess();
    return true;
}

void ConsoleSaveFileCommon::finalizeWrite()
{
    LockSaveAccess();
    mFileHeader.WriteHeader(mData);
    ReleaseSaveAccess();
}

bool ConsoleSaveFileCommon::doesFileExist(ConsoleSavePath path)
{
    LockSaveAccess();
    bool result = mFileHeader.fileExists(path.getPath());
    ReleaseSaveAccess();
    return result;
}

unsigned int ConsoleSaveFileCommon::getSizeOnDisk() { return mFileHeader.GetFileSize(); }
std::wstring ConsoleSaveFileCommon::getFilename() { return mFilename; }
std::vector<FileEntry*>* ConsoleSaveFileCommon::getFilesWithPrefix(const std::wstring& prefix) { return mFileHeader.getFilesWithPrefix(prefix); }
short ConsoleSaveFileCommon::getSaveVersion() { return mFileHeader.m_saveVersion; }
short ConsoleSaveFileCommon::getOriginalSaveVersion() { return mFileHeader.m_originalSaveVersion; }
void ConsoleSaveFileCommon::LockSaveAccess() { EnterCriticalSection(&mSaveAccessMutex); }
void ConsoleSaveFileCommon::ReleaseSaveAccess() { LeaveCriticalSection(&mSaveAccessMutex); }
ESavePlatform ConsoleSaveFileCommon::getSavePlatform() { return mFileHeader.m_savePlatform; }
bool ConsoleSaveFileCommon::isSaveEndianDifferent() { return mFileHeader.m_saveEndian != eByteOrder_BigEndian; }
void ConsoleSaveFileCommon::setLocalPlatform() { mFileHeader.m_saveEndian = eByteOrder_BigEndian; mFileHeader.m_savePlatform = eSavePlatform_WIIU; }
void ConsoleSaveFileCommon::setPlatform(ESavePlatform platform) { mFileHeader.m_savePlatform = platform; }
ByteOrder ConsoleSaveFileCommon::getSaveEndian() { return mFileHeader.m_saveEndian; }
ByteOrder ConsoleSaveFileCommon::getLocalEndian() { return eByteOrder_BigEndian; }
void ConsoleSaveFileCommon::setEndian(ByteOrder endian) { mFileHeader.m_saveEndian = endian; }
bool ConsoleSaveFileCommon::isLocalEndianDifferent(ESavePlatform platform) { return getLocalEndian() != FileHeader::getEndian(platform); }

void ConsoleSaveFileCommon::ConvertRegionFile(File regionFile)
{
    RegionFile region(this, &regionFile);
    bool compressed = true;
    int x;
    for (x = 0; x < 32; ++x)
    {
        int z;
        for (z = 0; z < 32; ++z)
        {
            DataInputStream* input = region.getChunkDataInputStream(x, z, &compressed);
            if (input != NULL)
            {
                arrayWithLength<unsigned char> data(0x100000, true);
                int length = input->read(data);
                input->close();
                input->deleteChildStream();
                delete input;
                DataOutputStream* output = region.getChunkDataOutputStream(x, z, compressed);
                output->write(data, 0, length);
                output->close();
                output->deleteChildStream();
                delete output;
                delete[] data.data;
            }
        }
    }
    region.writeAllOffsets();
}

void ConsoleSaveFileCommon::ConvertToLocalPlatform()
{
    if (getSavePlatform() == eSavePlatform_WIIU)
        return;
    std::vector<FileEntry*>* files = getFilesWithPrefix(L"");
    if (files != NULL)
    {
        std::vector<FileEntry*>::iterator it;
        for (it = files->begin(); it != files->end(); ++it)
        {
            FileEntry* entry = *it;
            if (entry->m_fileName.compare(entry->m_fileName.size() - 4, 4, L".mcr") == 0)
            {
                File file(entry->m_fileName);
                ConvertRegionFile(file);
            }
        }
    }
    setLocalPlatform();
}

unsigned int ConsoleSaveFileCommon::fjGetSaveThumbnailData(unsigned char* data, unsigned int size)
{
    std::memset(data, 0, size);
    CMinecraftApp::PngTagData imageData;
    g_pApp->CurrentImageTextData(&imageData);
    if ((imageData.flags & 0x02000000) == 0)
    {
        imageData.dataSize = 0;
        imageData.flags |= 0x02000000;
    }
    imageData.flags |= 0x08000000;
    g_pApp->SetExtraGameData(imageData.values[7], static_cast<eGameExtraData>(4), 0x881);
    g_pApp->SetExtraGameData(imageData.values[7], static_cast<eGameExtraData>(5), 0xb);

    unsigned int imageDataSize = g_pApp->CreateImageTextData(data, &imageData);

    CMinecraftApp::PngTagData parsedData;
    g_pApp->ParseImageTextData(data, imageDataSize, &parsedData);
    return imageDataSize;
}

int ConsoleSaveFileCommon::fjCountSavedChunks(int dimension)
{
    const std::wstring dimensions[] = {L"", L"DIM-1", L"DIM1"};
    const std::wstring& folder = dimensions[dimension];
    int savedChunks = 0;
    int regionX;
    for (regionX = -4; regionX < 4; ++regionX)
    {
        int regionZ;
        for (regionZ = -128; regionZ < 128; regionZ += 32)
        {
            File regionPath = RegionFileCache::fjGetFilepath(this, folder, regionX << 5, regionZ);
            if (fjHasRegionFile(regionPath))
            {
                RegionFile* regionFile = gRegionFileCache._getRegionFile(this, folder, regionX << 5, regionZ);
                if (regionFile != NULL)
                    savedChunks += regionFile->fjCountSavedChunks();
            }
        }
    }
    return savedChunks;
}
bool ConsoleSaveFileCommon::fjHasRegionFile(const File& file) { return mFileHeader.fileExists(file.getPath()); }
