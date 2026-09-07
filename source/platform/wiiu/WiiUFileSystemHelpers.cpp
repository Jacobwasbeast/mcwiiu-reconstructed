#include "platform/wiiu/WiiUFileSystemHelpers.hpp"

extern void setAltMainOKToRun();
extern void* GetCommandBlock();
extern void waitForAltMainThreadToComplete();

extern "C" int FSOpenDir(void* client, void* commandBlock, const char* path, int* directory,
                          unsigned int error);

static void* s_fileSystemClient = 0;

int OpenDir(const char* path, int* directory)
{
    setAltMainOKToRun();
    void* commandBlock = GetCommandBlock();
    int result = FSOpenDir(s_fileSystemClient, commandBlock, path, directory, 0);
    waitForAltMainThreadToComplete();
    return result;
}
