#include "game/data/LevelStorage.hpp"

LevelStorage::~LevelStorage()
{
}

ConsoleSaveFile* LevelStorage::getSaveFile()
{
    return NULL;
}

void LevelStorage::flushSaveFile(bool bForce)
{
}

int LevelStorage::getAuxValueForMap(PlayerUID uid, int dimension, int x, int z, int scale)
{
    return 0;
}
