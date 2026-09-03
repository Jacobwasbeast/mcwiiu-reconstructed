#include "platform/wiiu/WiiU_FileCache.hpp"

#include <cstring>
#include <map>

#include "game/core/CriticalSection.hpp"

extern "C" void *MEMAllocFromDefaultHeapEx(unsigned int size, int alignment);
extern "C" void MEMFreeToDefaultHeap(void *block);
extern "C" void *GetCommandBlock();
extern "C" int FSSetPosFile(void *client, void *commandBlock, int file, unsigned int position, unsigned int error);
extern "C" int FSReadFile(void *client, void *commandBlock, void *buffer, unsigned int count, unsigned int size, int file, unsigned int flags, unsigned int error);
extern "C" int FSCloseFile(void *client, void *commandBlock, int file, unsigned int error);
extern "C" unsigned int GetFileSize(int file, unsigned int *fileSizeHigh);

static std::map<int, WiiU_FileCache *> s_fileCaches;
static OSFastMutex s_fileCacheMutex;
static bool s_fileCacheMutexInitialized = false;
static void *s_fileSystemClient = 0;

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
