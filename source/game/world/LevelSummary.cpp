#include "game/world/LevelSummary.hpp"

LevelSummary::LevelSummary(LevelData* levelData, const std::wstring& levelId, const std::wstring& levelName, long long sizeOnDisk, bool requiresConversion)
    : mLevelId(levelId)
    , mLevelName(levelName)
    , mLastPlayed(levelData->getLastPlayed())
    , mSizeOnDisk(sizeOnDisk)
    , mRequiresConversion(requiresConversion)
    , mGameMode(levelData->getGameType())
    , mHardcore(levelData->isHardcore())
    , mCheats(levelData->getAllowCommands())
{
}

std::wstring LevelSummary::getLevelId()
{
    return mLevelId;
}

std::wstring LevelSummary::getLevelName()
{
    return mLevelName;
}

long long LevelSummary::getSizeOnDisk()
{
    return mSizeOnDisk;
}

long long LevelSummary::getLastPlayed()
{
    return mLastPlayed;
}

bool LevelSummary::isRequiresConversion()
{
    return mRequiresConversion;
}
