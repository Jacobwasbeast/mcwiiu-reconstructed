#include "game/data/ConsoleSaveFile.hpp"

#include <cstdlib>

static void* sSaveData = NULL;
static unsigned long sSaveDataSize = 0;

void* VirtualAlloc(void* address, unsigned long size, unsigned int, unsigned int)
{
    if (address == NULL)
    {
        sSaveDataSize = size;
        sSaveData = std::malloc(size);
    }
    return sSaveData;
}

int VirtualFree(void*, unsigned long, unsigned int)
{
    return 1;
}

ConsoleSaveFile::~ConsoleSaveFile()
{
}

void ConsoleSaveFile::tick()
{
}

unsigned char* ConsoleSaveFile::getWritePointer(FileEntry* entry)
{
    return NULL;
}

void ConsoleSaveFile::refreshFromLevelgen()
{
}
