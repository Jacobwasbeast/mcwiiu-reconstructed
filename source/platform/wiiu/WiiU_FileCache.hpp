#ifndef SOURCE_PLATFORM_WIIU_WIIU_FILECACHE_HPP
#define SOURCE_PLATFORM_WIIU_WIIU_FILECACHE_HPP

#include <stdint.h>
#include <coreinit/filesystem.h>

int SetFilePointer(int file, long distanceToMove, int* distanceToMoveHigh, unsigned int moveMethod);
int WriteFile(int file, const void* buffer, unsigned int bytesToWrite, unsigned int* bytesWritten, struct _OVERLAPPED* overlapped);
void WiiUFileCacheInitialise();

class WiiU_FileCache
{
    friend int SetFilePointer(int file, long distanceToMove, int* distanceToMoveHigh, unsigned int moveMethod);
private:
    unsigned char *m_cache;
    int m_file;
    unsigned int m_fileSize;
    int m_filePos;
    int m_cacheLength;
    int m_cachePos;

public:
    WiiU_FileCache(int file);
    ~WiiU_FileCache();
    static WiiU_FileCache *GetFileCache(int file);
    int bytesAvailableInCache();
    void fillCache();
    unsigned int Read(void *buffer, unsigned int size);
    unsigned int SetFilePos(int position);
    static void CreateFileCache(int file);
    bool Close();
    static bool DeleteFileCache(int file);
};

#endif
