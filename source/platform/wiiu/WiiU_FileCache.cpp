#include "platform/wiiu/WiiU_FileCache.hpp"
#include "game/world/block/BlockId.hpp"

#include <cstring>
#include <cstdlib>
#include <map>

#include "game/core/CriticalSection.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"
#include "platform/wiiu/CafeThreadLocal.hpp"
#include "platform/wiiu/WiiU_FileSystem.hpp"

#include <coreinit/memdefaultheap.h>
#include <coreinit/thread.h>

extern void setAltMainOKToRun();
extern void waitForAltMainThreadToComplete();

static void *s_virtualMemory = 0;
static unsigned long s_virtualMemorySize = 0;
static std::map<int, WiiU_FileCache *> s_fileCaches;
static OSFastMutex s_fileCacheMutex;
static bool s_fileCacheMutexInitialized = false;
unsigned char *s_writeBuffer = 0;
OSFastMutex s_writeMutex;
OSFastMutex s_commandBlockMutex;
static std::map<int, FSCmdBlock *> s_commandBlocks;
static int s_nextCommandBlockId = 1;
static std::map<OSThread*, int> s_commandBlockIds;

static int& currentCommandBlockId()
{
    return CafeThreadLocal::slot(s_commandBlockIds);
}

struct _OVERLAPPED;

FSCmdBlock *GetCommandBlock()
{
    EnterCriticalSection(&s_commandBlockMutex);
    int& commandBlockId = currentCommandBlockId();
    if (commandBlockId == BlockId::AIR)
    {
        commandBlockId = s_nextCommandBlockId++;
    }

    std::map<int, FSCmdBlock *>::iterator it = s_commandBlocks.find(commandBlockId);
    if (it == s_commandBlocks.end())
    {
        FSCmdBlock *commandBlock = static_cast<FSCmdBlock *>(std::malloc(sizeof(FSCmdBlock)));
        FSInitCmdBlock(commandBlock);
        s_commandBlocks[commandBlockId] = commandBlock;
        LeaveCriticalSection(&s_commandBlockMutex);
        return commandBlock;
    }

    FSCmdBlock *commandBlock = it->second;
    LeaveCriticalSection(&s_commandBlockMutex);
    return commandBlock;
}

unsigned int GetFileSize(int file, unsigned int *fileSizeHigh)
{
    (void)fileSizeHigh;
    FSStat stat;
    setAltMainOKToRun();
    FSCmdBlock *commandBlock = GetCommandBlock();
    FSGetStatFile(GetWiiUFileSystemClient(), commandBlock, file, &stat, FS_ERROR_FLAG_NONE);
    waitForAltMainThreadToComplete();
    return stat.size;
}

unsigned int ReadFile(int file, void *buffer, unsigned int bytesToRead,
                      unsigned int *bytesRead, _OVERLAPPED *overlapped)
{
    (void)overlapped;
    setAltMainOKToRun();
    WiiU_FileCache *cache = WiiU_FileCache::GetFileCache(file);
    unsigned int result = 0;
    if (cache != 0)
    {
        result = cache->Read(buffer, bytesToRead);
        if (bytesRead != 0)
        {
            *bytesRead = result;
        }
    }
    waitForAltMainThreadToComplete();
    return (result >> 31) ^ 1;
}

int WriteFile(int file, const void *buffer, unsigned int bytesToWrite,
              unsigned int *bytesWritten, _OVERLAPPED *overlapped)
{
    (void)overlapped;
    setAltMainOKToRun();
    FSStatus result;
    if ((bytesToWrite & 0x3fu) == 0)
    {
        FSCmdBlock *commandBlock = GetCommandBlock();
        result = FSWriteFile(GetWiiUFileSystemClient(), commandBlock, const_cast<unsigned char *>(static_cast<const unsigned char *>(buffer)), 1, bytesToWrite, file, 0, FS_ERROR_FLAG_ALL);
    }
    else
    {
        if (s_writeBuffer == 0)
        {
            waitForAltMainThreadToComplete();
            return 0;
        }
        EnterCriticalSection(&s_writeMutex);
        std::memcpy(s_writeBuffer, buffer, bytesToWrite);
        FSCmdBlock *commandBlock = GetCommandBlock();
        result = FSWriteFile(GetWiiUFileSystemClient(), commandBlock, s_writeBuffer, 1, bytesToWrite, file, 0, FS_ERROR_FLAG_ALL);
        LeaveCriticalSection(&s_writeMutex);
    }
    if (result >= 0 && bytesWritten != 0)
    {
        *bytesWritten = static_cast<unsigned int>(result);
    }
    waitForAltMainThreadToComplete();
    return result >= 0 ? 1 : 0;
}

int SetFilePointer(int file, long distanceToMove, int *distanceToMoveHigh, unsigned int moveMethod)
{
    (void)distanceToMoveHigh;
    setAltMainOKToRun();
    WiiU_FileCache *cache = WiiU_FileCache::GetFileCache(file);
    int position = static_cast<int>(distanceToMove);
    if (moveMethod == 2)
    {
        position += static_cast<int>(cache->m_fileSize);
    }
    else if (moveMethod == 1)
    {
        position += cache->m_filePos;
    }
    int result = cache->SetFilePos(position);
    waitForAltMainThreadToComplete();
    return result;
}

unsigned int GetFileAttributesA(const char *fileName)
{
    char path[256];
    FSStat stat;
    setAltMainOKToRun();
    getFinalFilePath(path, fileName);
    FSCmdBlock *commandBlock = GetCommandBlock();
    FSStatus result = FSGetStat(GetWiiUFileSystemClient(), commandBlock, path, &stat, FS_ERROR_FLAG_NOT_FOUND);
    waitForAltMainThreadToComplete();
    if (result == -6)
    {
        return 0xffffffffu;
    }
    return (stat.flags & FS_STAT_DIRECTORY) != 0 ? 0x10 : 0x80;
}

WiiU_FileCache::WiiU_FileCache(int file)
    : m_cache(0), m_file(file), m_fileSize(0), m_filePos(0), m_cacheLength(0), m_cachePos(0)
{
    m_cache = (unsigned char *)MEMAllocFromDefaultHeapEx(FILE_CACHE_BUFFER_SIZE, 64);
    m_fileSize = GetFileSize(file, 0);
}

WiiU_FileCache::~WiiU_FileCache()
{
    if (m_cache != 0)
    {
        MEMFreeToDefaultHeap(m_cache);
    }
}

WiiU_FileCache *WiiU_FileCache::GetFileCache(int file)
{
    EnterCriticalSection(&s_fileCacheMutex);
    WiiU_FileCache *cache = s_fileCaches[file];
    LeaveCriticalSection(&s_fileCacheMutex);
    return cache;
}

int WiiU_FileCache::bytesAvailableInCache()
{
    int cacheEnd = m_cachePos + m_cacheLength;
    if (m_cachePos <= m_filePos && m_filePos < cacheEnd)
    {
        return cacheEnd - m_filePos;
    }
    return 0;
}

void WiiU_FileCache::fillCache()
{
    FSCmdBlock *commandBlock = GetCommandBlock();
    FSStatus result = FSSetPosFile(GetWiiUFileSystemClient(), commandBlock, m_file, (unsigned int)m_filePos, FS_ERROR_FLAG_ALL);
    if (result == 0)
    {
        commandBlock = GetCommandBlock();
        result = FSReadFile(GetWiiUFileSystemClient(), commandBlock, m_cache, 1, FILE_CACHE_BUFFER_SIZE, m_file, 0, FS_ERROR_FLAG_ALL);
        if (result >= 0)
        {
            m_cacheLength = result;
            m_cachePos = m_filePos;
            return;
        }
        m_cacheLength = 0;
    }
}

unsigned int WiiU_FileCache::Read(void *buffer, unsigned int size)
{
    unsigned int total = 0;
    int available = bytesAvailableInCache();
    while (total < size)
    {
        if (available == 0)
        {
            fillCache();
            available = bytesAvailableInCache();
            if (available == 0)
            {
                return total;
            }
        }
        unsigned int amount = size - total;
        if (available < (int)amount)
        {
            amount = (unsigned int)available;
        }
        std::memcpy(buffer, m_cache + (m_filePos - m_cachePos), amount);
        m_filePos += (int)amount;
        buffer = (unsigned char *)buffer + amount;
        total += amount;
        available = bytesAvailableInCache();
    }
    return total;
}

unsigned int WiiU_FileCache::SetFilePos(int position)
{
    if (position >= 0 && (unsigned int)position < m_fileSize)
    {
        unsigned int cacheBase = (unsigned int)m_filePos & 0xffff0000u;
        if (cacheBase != (unsigned int)m_cachePos)
        {
            m_filePos = (int)cacheBase;
            fillCache();
        }
        m_filePos = position;
        return 1;
    }
    return 0;
}

void WiiU_FileCache::CreateFileCache(int file)
{
    if (!s_fileCacheMutexInitialized)
    {
        InitializeCriticalSection(&s_fileCacheMutex);
        s_fileCacheMutexInitialized = true;
    }
    EnterCriticalSection(&s_fileCacheMutex);
    WiiU_FileCache *cache = new WiiU_FileCache(file);
    s_fileCaches[file] = cache;
    LeaveCriticalSection(&s_fileCacheMutex);
}

bool WiiU_FileCache::Close()
{
    FSCmdBlock *commandBlock = GetCommandBlock();
    FSStatus result = FSCloseFile(GetWiiUFileSystemClient(), commandBlock, m_file, FS_ERROR_FLAG_ALL);
    return result == 0;
}

bool WiiU_FileCache::DeleteFileCache(int file)
{
    EnterCriticalSection(&s_fileCacheMutex);
    WiiU_FileCache *cache = GetFileCache(file);
    if (cache == 0)
    {
        LeaveCriticalSection(&s_fileCacheMutex);
        return false;
    }
    bool result = cache->Close();
    delete cache;
    s_fileCaches.erase(file);
    LeaveCriticalSection(&s_fileCacheMutex);
    return result;
}

void *VirtualAlloc(void *address, unsigned long size, unsigned int, unsigned int)
{
    if (address == 0)
    {
        s_virtualMemorySize = size;
        s_virtualMemory = std::malloc(size);
    }
    return s_virtualMemory;
}

int VirtualFree(void *, unsigned long, unsigned int)
{
    return 1;
}
