#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"

#include <coreinit/filesystem.h>

#include "platform/wiiu/WiiU_FileCache.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"
#include "platform/wiiu/WiiU_FileSystem.hpp"

extern void setAltMainOKToRun();
extern void waitForAltMainThreadToComplete();
extern FSCmdBlock* GetCommandBlock();

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
    FSCmdBlock* commandBlock = GetCommandBlock();
    FSFileHandle file = static_cast<FSFileHandle>(-1);

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

        FSOpenFile(GetWiiUFileSystemClient(), commandBlock, path, mode, &file,
                   static_cast<FSErrorFlag>(8));
        if (file != static_cast<FSFileHandle>(-1))
            WiiU_FileCache::CreateFileCache(static_cast<int>(file));
    }

    waitForAltMainThreadToComplete();
    return static_cast<int>(file);
}

bool CreateDirectoryA(const char* pathName, void* securityAttributes)
{
    (void)securityAttributes;

    setAltMainOKToRun();
    char path[256];
    getFinalFilePath(path, pathName);
    FSCmdBlock* commandBlock = GetCommandBlock();
    FSStatus result = FSMakeDir(GetWiiUFileSystemClient(), commandBlock, path,
                                FS_ERROR_FLAG_NONE);
    waitForAltMainThreadToComplete();
    return result == 0;
}

bool CloseDir(int* directory)
{
    setAltMainOKToRun();
    FSCmdBlock* commandBlock = GetCommandBlock();
    FSStatus result = FSCloseDir(GetWiiUFileSystemClient(), commandBlock,
                                 static_cast<FSDirectoryHandle>(*directory), FS_ERROR_FLAG_NONE);
    waitForAltMainThreadToComplete();
    return result == 0;
}

void CloseHandle(int file)
{
    WiiU_FileCache::DeleteFileCache(file);
}
