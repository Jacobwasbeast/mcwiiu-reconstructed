#include "platform/wiiu/WiiUFileSystemHelpers.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

extern void setAltMainOKToRun();
extern void* GetCommandBlock();
extern void waitForAltMainThreadToComplete();

int OpenDir(const char* path, int* directory)
{
    setAltMainOKToRun();
    FSCmdBlock* commandBlock = static_cast<FSCmdBlock*>(GetCommandBlock());
    FSDirectoryHandle handle = directory != 0 ? static_cast<FSDirectoryHandle>(*directory) : 0;
    FSStatus result = FSOpenDir(GetWiiUFileSystemClient(), commandBlock, path, &handle,
                                FS_ERROR_FLAG_NONE);
    if (directory != 0)
    {
        *directory = static_cast<int>(handle);
    }
    waitForAltMainThreadToComplete();
    return result == 0;
}
