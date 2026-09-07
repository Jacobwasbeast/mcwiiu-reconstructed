#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"

#include "platform/wiiu/WiiU_FileCache.hpp"

extern void setAltMainOKToRun();
extern void waitForAltMainThreadToComplete();
extern void getFinalFilePath(char (&path)[256], const char* fileName);
extern void* GetCommandBlock();

extern "C" int FSOpenFile(void* client, void* commandBlock, const char* path, const char* mode,
                           int* file, unsigned int error);
extern "C" int FSMakeDir(void* client, void* commandBlock, const char* path, unsigned int error);
extern "C" int FSCloseDir(void* client, void* commandBlock, int directory, unsigned int error);

static void* s_fileSystemClient = 0;

int CreateFileA(const char* fileName, unsigned int desiredAccess, unsigned int shareMode,
               void* securityAttributes, unsigned int creationDisposition,
               unsigned int flagsAndAttributes, int templateFile)
{
    (void)shareMode;
    (void)securityAttributes;
    (void)flagsAndAttributes;
    (void)templateFile;

    setAltMainOKToRun();
    char path[256];
    getFinalFilePath(path, fileName);
    void* commandBlock = GetCommandBlock();
    int file = -1;

    if (creationDisposition != 0)
    {
        const char* mode = "a+";
        if (creationDisposition < 3)
            mode = "w+";
        else if (creationDisposition == 3 && desiredAccess == 0x80000000U)
            mode = "r";
        else if (creationDisposition > 4)
        {
            if (creationDisposition == 5)
                mode = "w+";
            else
            {
                waitForAltMainThreadToComplete();
                return file;
            }
        }

        FSOpenFile(s_fileSystemClient, commandBlock, path, mode, &file, 8);
        if (file != -1)
            WiiU_FileCache::CreateFileCache(file);
    }

    waitForAltMainThreadToComplete();
    return file;
}

bool CreateDirectoryA(const char* pathName, void* securityAttributes)
{
    (void)securityAttributes;

    setAltMainOKToRun();
    char path[256];
    getFinalFilePath(path, pathName);
    void* commandBlock = GetCommandBlock();
    int result = FSMakeDir(s_fileSystemClient, commandBlock, path, 0);
    waitForAltMainThreadToComplete();
    return result == 0;
}

bool CloseDir(int* directory)
{
    setAltMainOKToRun();
    void* commandBlock = GetCommandBlock();
    int result = FSCloseDir(s_fileSystemClient, commandBlock, *directory, 0);
    waitForAltMainThreadToComplete();
    return result == 0;
}

void CloseHandle(int file)
{
    WiiU_FileCache::DeleteFileCache(file);
}
