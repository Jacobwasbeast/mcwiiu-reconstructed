#include "game/data/DevSaveDataManager__NamedPositionOrVolume.hpp"

DevSaveDataManager::NamedPositionOrVolume::NamedPositionOrVolume(std::string name, const BlockPos& position)
{
    mName = name;
    mIsVolume = false;
    mPositions[0] = position;
}

DevSaveDataManager::NamedPositionOrVolume::NamedPositionOrVolume(std::string name, const BlockPos& from, const BlockPos& to)
{
    mName = name;
    mIsVolume = true;
    mPositions[0] = from;
    mPositions[1] = to;
}
