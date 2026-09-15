#include "game/data/ConsoleSaveFile.hpp"

ConsoleSaveFile::~ConsoleSaveFile()
{
}

void ConsoleSaveFile::tick()
{
    return;
}

unsigned char* ConsoleSaveFile::getWritePointer(FileEntry* entry)
{
    return 0;
}

void ConsoleSaveFile::refreshFromLevelgen()
{
    return;
}
