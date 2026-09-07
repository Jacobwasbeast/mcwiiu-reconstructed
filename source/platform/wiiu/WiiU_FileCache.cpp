#include "platform/wiiu/WiiU_FileCache.hpp"

#include <cstring>
#include <cstdlib>
#include <map>

#include "game/core/CriticalSection.hpp"

struct FSDirEntry;

extern "C" void *MEMAllocFromDefaultHeapEx(unsigned int size, int alignment);
extern "C" void MEMFreeToDefaultHeap(void *block);
extern "C" void FSInitCmdBlock(void *commandBlock);
extern "C" int FSSetPosFile(void *client, void *commandBlock, int file, unsigned int position, unsigned int error);
extern "C" int FSReadFile(void *client, void *commandBlock, void *buffer, unsigned int count, unsigned int size, int file, unsigned int flags, unsigned int error);
extern "C" int FSCloseFile(void *client, void *commandBlock, int file, unsigned int error);
extern "C" int FSGetStatFile(void *client, void *commandBlock, int file, void *stat, unsigned int error);
extern "C" int FSReadDir(void *client, void *commandBlock, int file, void *entry, unsigned int error);
extern "C" int FSGetStat(void *client, void *commandBlock, const char *path, void *stat, unsigned int error);

extern void setAltMainOKToRun();
extern void waitForAltMainThreadToComplete();
extern void getFinalFilePath(char (&path)[256], const char *fileName);

static std::map<int, WiiU_FileCache *> s_fileCaches;
static OSFastMutex s_fileCacheMutex;
static bool s_fileCacheMutexInitialized = false;
static void *s_fileSystemClient = 0;
static OSFastMutex s_commandBlockMutex;
static std::map<int, void *> s_commandBlocks;
static int s_nextCommandBlockId = 1;
static __thread int s_commandBlockId;

void *GetCommandBlock()
{
    EnterCriticalSection(&s_commandBlockMutex);
    if (s_commandBlockId == 0)
    {
        s_commandBlockId = s_nextCommandBlockId++;
    }

    std::map<int, void *>::iterator it = s_commandBlocks.find(s_commandBlockId);
    if (it == s_commandBlocks.end())
    {
        void *commandBlock = std::malloc(0xa80);
        FSInitCmdBlock(commandBlock);
        s_commandBlocks[s_commandBlockId] = commandBlock;
        LeaveCriticalSection(&s_commandBlockMutex);
        return commandBlock;
    }

    void *commandBlock = it->second;
    LeaveCriticalSection(&s_commandBlockMutex);
    return commandBlock;
}

unsigned int GetFileSize(int file, unsigned int *fileSizeHigh)
{
    (void)fileSizeHigh;
    unsigned int stat[25];
    setAltMainOKToRun();
    void *commandBlock = GetCommandBlock();
    FSGetStatFile(s_fileSystemClient, commandBlock, file, stat, 0);
    waitForAltMainThreadToComplete();
    return stat[4];
}

unsigned int ReadFile(int file, void *buffer, unsigned int bytesToRead,
                      unsigned int *bytesRead, void *overlapped)
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

bool ReadDir(int *file, FSDirEntry *entry)
{
    setAltMainOKToRun();
    void *commandBlock = GetCommandBlock();
    int result = FSReadDir(s_fileSystemClient, commandBlock, *file, entry, 0);
    waitForAltMainThreadToComplete();
    return result == 0;
}

unsigned int GetFileAttributesA(const char *fileName)
{
    char path[256];
    unsigned int stat[25];
    setAltMainOKToRun();
    getFinalFilePath(path, fileName);
    void *commandBlock = GetCommandBlock();
    int result = FSGetStat(s_fileSystemClient, commandBlock, path, stat, 8);
    waitForAltMainThreadToComplete();
    if (result == -6)
    {
        return 0xffffffffu;
    }
    return (stat[0] & 0x80000000u) != 0 ? 0x10 : 0x80;
}

double RadianToDegree(float radians)
{
    return static_cast<double>(radians * 180.0f / 3.141590118408203125f);
}

WiiU_FileCache::WiiU_FileCache(int file)
    : m_cache(0), m_file(file), m_fileSize(0), m_filePos(0), m_cacheLength(0), m_cachePos(0)
{
    m_cache = (unsigned char *)MEMAllocFromDefaultHeapEx(0x10000, 64);
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
    void *commandBlock = GetCommandBlock();
    int result = FSSetPosFile(s_fileSystemClient, commandBlock, m_file, (unsigned int)m_filePos, 0xffffffffu);
    if (result == 0)
    {
        commandBlock = GetCommandBlock();
        result = FSReadFile(s_fileSystemClient, commandBlock, m_cache, 1, 0x10000, m_file, 0, 0xffffffffu);
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
    void *commandBlock = GetCommandBlock();
    int result = FSCloseFile(s_fileSystemClient, commandBlock, m_file, 0xffffffffu);
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
