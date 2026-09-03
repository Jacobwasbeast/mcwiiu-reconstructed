#include "game/data/ConsoleSaveFileCommon.hpp"

#include <cstring>

#include "game/core/File.hpp"
#include "game/core/RegionFile.hpp"
#include "game/core/System.hpp"

ConsoleSaveFileCommon::ConsoleSaveFileCommon()
    : mFilename(), mData(NULL), mDataSize(0)
{
    InitializeCriticalSectionAndSpinCount(&mSaveAccessMutex, 0x1400);
}

ConsoleSaveFileCommon::~ConsoleSaveFileCommon()
{
    DeleteCriticalSection(&mSaveAccessMutex);
    mData = NULL;
    mDataSize = 0;
}

void ConsoleSaveFileCommon::_initSaveFromData(const std::wstring& filename, unsigned int dataSize, void* data, ESavePlatform platform)
{
    mFilename = filename;
    mDataSize = dataSize;
    setPlatform(platform);
    if (mData == NULL)
        mData = new unsigned char[dataSize];
    if (data != NULL && dataSize != 0)
        std::memcpy(mData, data, dataSize);
    if (dataSize == 0)
    {
        mFileHeader.WriteHeader(mData);
        return;
    }
    if (data == NULL)
        std::memset(mData, 0, dataSize);
    mFileHeader.ReadHeader(mData, platform);
}

void ConsoleSaveFileCommon::_deleteFile(FileEntry* entry)
{
    if (entry == NULL || mData == NULL)
        return;
    LockSaveAccess();
    unsigned int source = entry->m_startOffset + entry->m_fileSize;
    unsigned int destination = entry->m_startOffset;
    unsigned int end = mFileHeader.GetStartOfNextData();
    if (source < end)
        std::memmove((unsigned char*)mData + destination, (unsigned char*)mData + source, end - source);
    mFileHeader.RemoveFile(entry);
    mFileHeader.WriteHeader(mData);
    ReleaseSaveAccess();
}

void ConsoleSaveFileCommon::MoveDataBeyond(FileEntry* entry, unsigned int size)
{
    if (entry == NULL || mData == NULL || size == 0)
        return;
    unsigned int start = entry->m_startOffset + entry->m_fileSize;
    unsigned int end = mFileHeader.GetStartOfNextData();
    if (end > start)
        std::memmove((unsigned char*)mData + start + size, (unsigned char*)mData + start, end - start);
    mFileHeader.AdjustStartOffsets(entry, size, false);
}

void ConsoleSaveFileCommon::PrepareForWrite(FileEntry* entry, unsigned int size)
{
    if (entry == NULL)
        return;
    unsigned int required = entry->m_filePointer + size;
    if (required > entry->m_fileSize)
    {
        unsigned int delta = required - entry->m_fileSize;
        MoveDataBeyond(entry, delta);
        entry->m_fileSize += delta;
    }
}

bool ConsoleSaveFileCommon::_writeFile(FileEntry* entry, const void* buffer, unsigned int size, unsigned int* written)
{
    if (mData == NULL || entry == NULL)
        return false;
    PrepareForWrite(entry, size);
    std::memcpy((unsigned char*)mData + entry->m_startOffset + entry->m_filePointer, buffer, size);
    entry->m_filePointer += size;
    if (written != NULL)
        *written = size;
    entry->m_timestamp = (long long)System::currentRealTimeMillis();
    return true;
}

bool ConsoleSaveFileCommon::_zeroFile(FileEntry* entry, unsigned int size, unsigned int* written)
{
    if (mData == NULL || entry == NULL)
        return false;
    PrepareForWrite(entry, size);
    std::memset((unsigned char*)mData + entry->m_startOffset + entry->m_filePointer, 0, size);
    entry->m_filePointer += size;
    if (written != NULL)
        *written = size;
    entry->m_timestamp = (long long)System::currentRealTimeMillis();
    return true;
}

bool ConsoleSaveFileCommon::_readFile(FileEntry* entry, void* buffer, unsigned int size, unsigned int* read)
{
    if (mData == NULL || entry == NULL)
        return false;
    unsigned int available = entry->m_fileSize > entry->m_filePointer ? entry->m_fileSize - entry->m_filePointer : 0;
    if (size > available)
        size = available;
    if (size != 0)
        std::memcpy(buffer, (unsigned char*)mData + entry->m_startOffset + entry->m_filePointer, size);
    entry->m_filePointer += size;
    if (read != NULL)
        *read = size;
    entry->m_timestamp = (long long)System::currentRealTimeMillis();
    return true;
}

bool ConsoleSaveFileCommon::closeHandle(FileEntry*)
{
    ReleaseSaveAccess();
    return true;
}

void ConsoleSaveFileCommon::finalizeWrite()
{
    if (mData != NULL)
        mFileHeader.WriteHeader(mData);
}

bool ConsoleSaveFileCommon::doesFileExist(ConsoleSavePath path)
{
    LockSaveAccess();
    bool result = mFileHeader.fileExists(path.getPath());
    ReleaseSaveAccess();
    return result;
}

unsigned int ConsoleSaveFileCommon::getSizeOnDisk()
{
    return mFileHeader.GetFileSize();
}

std::wstring ConsoleSaveFileCommon::getFilename()
{
    return mFilename;
}

std::vector<FileEntry*>* ConsoleSaveFileCommon::getFilesWithPrefix(const std::wstring& prefix)
{
    return mFileHeader.getFilesWithPrefix(prefix);
}

short ConsoleSaveFileCommon::getSaveVersion()
{
    return mFileHeader.m_saveVersion;
}

short ConsoleSaveFileCommon::getOriginalSaveVersion()
{
    return mFileHeader.m_originalSaveVersion;
}

void ConsoleSaveFileCommon::LockSaveAccess()
{
    EnterCriticalSection(&mSaveAccessMutex);
}

void ConsoleSaveFileCommon::ReleaseSaveAccess()
{
    LeaveCriticalSection(&mSaveAccessMutex);
}

ESavePlatform ConsoleSaveFileCommon::getSavePlatform()
{
    return mFileHeader.m_savePlatform;
}

bool ConsoleSaveFileCommon::isSaveEndianDifferent()
{
    return getSaveEndian() != getLocalEndian();
}

void ConsoleSaveFileCommon::setLocalPlatform()
{
    setPlatform(eSavePlatform_WIIU);
    setEndian(eByteOrder_BigEndian);
}

void ConsoleSaveFileCommon::setPlatform(ESavePlatform platform)
{
    mFileHeader.m_savePlatform = platform;
}

ByteOrder ConsoleSaveFileCommon::getSaveEndian()
{
    return mFileHeader.m_saveEndian;
}

ByteOrder ConsoleSaveFileCommon::getLocalEndian()
{
    return eByteOrder_BigEndian;
}

void ConsoleSaveFileCommon::setEndian(ByteOrder endian)
{
    mFileHeader.m_saveEndian = endian;
}

bool ConsoleSaveFileCommon::isLocalEndianDifferent(ESavePlatform platform)
{
    return FileHeader::getEndian(platform) != getLocalEndian();
}

void ConsoleSaveFileCommon::ConvertRegionFile(File regionFile)
{
    RegionFile region((ConsoleSaveFile*)this, &regionFile);
    bool compressed = true;
    for (int x = 0; x < 32; ++x)
    {
        for (int z = 0; z < 32; z += 2)
        {
            DataInputStream* input = region.getChunkDataInputStream(x, z, &compressed);
            if (input != NULL)
            {
                arrayWithLength<unsigned char> data(0x100000, true);
                int length = input->read(data);
                input->close();
                input->deleteChildStream();
                DataOutputStream* output = region.getChunkDataOutputStream(x, z, compressed);
                output->write(data, 0, length);
                output->close();
                output->deleteChildStream();
            }
            input = region.getChunkDataInputStream(x, z + 1, &compressed);
            if (input != NULL)
            {
                arrayWithLength<unsigned char> data(0x100000, true);
                int length = input->read(data);
                input->close();
                input->deleteChildStream();
                DataOutputStream* output = region.getChunkDataOutputStream(x, z + 1, compressed);
                output->write(data, 0, length);
                output->close();
                output->deleteChildStream();
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
        for (std::vector<FileEntry*>::iterator it = files->begin(); it != files->end(); ++it)
        {
            FileEntry* entry = *it;
            if (entry->m_fileName.compare(entry->m_fileName.size() - 4, 4, L".mcr") == 0)
            {
                File file(entry->m_fileName);
                ConvertRegionFile(file);
            }
        }
        delete files;
    }
    finalizeWrite();
}

unsigned int ConsoleSaveFileCommon::fjGetSaveThumbnailData(unsigned char* data, unsigned int size)
{
    if (data != NULL && size != 0)
        std::memset(data, 0, size);
    return 0;
}

int ConsoleSaveFileCommon::fjCountSavedChunks(int)
{
    return 0;
}

bool ConsoleSaveFileCommon::fjHasRegionFile(const File& file)
{
    return mFileHeader.fileExists(file.getPath());
}
