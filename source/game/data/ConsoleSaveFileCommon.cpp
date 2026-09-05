#include "game/data/ConsoleSaveFileCommon.hpp"

#include <cstring>

#include "game/core/Compression.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/File.hpp"
#include "game/core/RegionFile.hpp"
#include "game/core/System.hpp"

void PIXBeginNamedEvent(float color, const char* format, ...);
void PIXEndNamedEvent();

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
    setPlatform(platform);
    if (dataSize == 0)
    {
        mFileHeader.WriteHeader(mData);
        return;
    }
    if (mData == NULL)
        mData = new unsigned char[dataSize];
    if (data == NULL)
        std::memset(mData, 0, dataSize);
    else
        std::memcpy(mData, data, dataSize);
    if (data == NULL)
        mFileHeader.WriteHeader(mData);
    else
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
    unsigned int source = entry->m_startOffset + entry->m_fileSize;
    unsigned int end = mFileHeader.GetStartOfNextData();
    if ((size & 0xfff) == 0)
    {
        unsigned int destination = source + size;
        while (source < end)
        {
            unsigned int count = end - source;
            if (count > 0x1000)
                count = 0x1000;
            std::memmove(static_cast<unsigned char*>(mData) + destination,
                         static_cast<unsigned char*>(mData) + source, count);
            source += count;
            destination += count;
        }
    }
    else
    {
        std::memmove(static_cast<unsigned char*>(mData) + source + size,
                     static_cast<unsigned char*>(mData) + source, end - source);
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
    if (mData == NULL)
        return false;
    LockSaveAccess();
    PrepareForWrite(entry, size);
    std::memcpy(static_cast<unsigned char*>(mData) + entry->m_filePointer, buffer, size);
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
bool ConsoleSaveFileCommon::isLocalEndianDifferent(ESavePlatform platform) { return FileHeader::getEndian(platform) != getLocalEndian(); }

void ConsoleSaveFileCommon::ConvertRegionFile(File regionFile)
{
    RegionFile region(this, &regionFile);
    bool compressed = true;
    int x;
    for (x = 0; x < 32; ++x)
    {
        int z;
        for (z = 0; z < 32; z += 2)
        {
            int index;
            for (index = z; index <= z + 1; ++index)
            {
                DataInputStream* input = region.getChunkDataInputStream(x, index, &compressed);
                if (input != NULL)
                {
                    arrayWithLength<unsigned char> data(0x100000, true);
                    int length = input->read(data);
                    input->close();
                    input->deleteChildStream();
                    delete input;
                    DataOutputStream* output = region.getChunkDataOutputStream(x, index, compressed);
                    output->write(data, 0, length);
                    output->close();
                    output->deleteChildStream();
                    delete output;
                    delete[] data.data;
                }
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
    return 0;
}

int ConsoleSaveFileCommon::fjCountSavedChunks(int) { return 0; }
bool ConsoleSaveFileCommon::fjHasRegionFile(const File& file) { return mFileHeader.fileExists(file.getPath()); }
