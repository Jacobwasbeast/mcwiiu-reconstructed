#include "game/world/LevelSettings.hpp"
#include "game/world/LevelData.hpp"
#include "game/core/SuperflatConfig.hpp"

LevelSettings::LevelSettings(long long seed, const GameType* gameType,
    bool generateMapFeatures, bool hardcore, bool allowCommands,
    LevelType* levelType, int cloudHeight, int xzSize, int hellScale,
    bool modernEnd, unsigned char biomeScale, int biomeCentreXChunk,
    int biomeCentreZChunk)
{
    _init(seed, gameType, generateMapFeatures, hardcore, allowCommands,
          levelType, cloudHeight, xzSize, hellScale, modernEnd, biomeScale,
          biomeCentreXChunk, biomeCentreZChunk);
}

LevelSettings::LevelSettings(LevelData* levelData)
{
    _init(levelData->getSeed(), levelData->getGameType(),
          levelData->isGenerateMapFeatures(), levelData->isHardcore(),
          levelData->getAllowCommands(), levelData->getGeneratorType(),
          levelData->getCloudHeight(), levelData->getXZSize(),
          levelData->getHellScale(), levelData->getModernEnd(),
          static_cast<unsigned char>(levelData->getBiomeScale()),
          levelData->getBiomeCentreXChunk(), levelData->getBiomeCentreZChunk());
}

void LevelSettings::_init(long long seed, const GameType* gameType,
    bool generateMapFeatures, bool hardcore, bool allowCommands,
    LevelType* levelType, int cloudHeight, int xzSize, int hellScale,
    bool modernEnd, unsigned char biomeScale, int biomeCentreXChunk,
    int biomeCentreZChunk)
{
    mSeed = seed;
    mGameType = gameType;
    mGenerateMapFeatures = generateMapFeatures;
    mHardcore = hardcore;
    mCloudHeight = cloudHeight;
    mUseNewSeaLevel = allowCommands;
    mLevelType = levelType;
    mAllowCommands = false;
    mStartingBonusItems = false;
    mLevelTypeOptions = 0;
    mXZSize = xzSize;
    mHellScale = hellScale;
    mModernEnd = modernEnd;
    mBiomeScale = biomeScale;
    mBiomeCentreXChunk = biomeCentreXChunk;
    mBiomeCentreZChunk = biomeCentreZChunk;
}

void LevelSettings::enableStartingBonusItems()
{
    mStartingBonusItems = true;
}

void LevelSettings::enableSinglePlayerCommands()
{
    mAllowCommands = true;
}

void LevelSettings::setLevelTypeOptions(SuperflatConfig* options)
{
    mLevelTypeOptions = options;
}

bool LevelSettings::hasStartingBonusItems() const
{
    return mStartingBonusItems;
}

long long LevelSettings::getSeed() const
{
    return mSeed;
}

const GameType* LevelSettings::getGameType() const
{
    return mGameType;
}

bool LevelSettings::isHardcore() const
{
    return mHardcore;
}

LevelType* LevelSettings::getLevelType() const
{
    return mLevelType;
}

bool LevelSettings::getAllowCommands() const
{
    return mAllowCommands;
}

bool LevelSettings::isGenerateMapFeatures() const
{
    return mGenerateMapFeatures;
}

int LevelSettings::getCloudHeight() const
{
    return mCloudHeight;
}

bool LevelSettings::useNewSeaLevel() const
{
    return mUseNewSeaLevel;
}

int LevelSettings::getXZSize() const
{
    return mXZSize;
}

int LevelSettings::getHellScale() const
{
    return mHellScale;
}

bool LevelSettings::getModernEnd() const
{
    return mModernEnd;
}

unsigned char LevelSettings::getBiomeScale() const
{
    return mBiomeScale;
}

int LevelSettings::getBiomeCentreXChunk() const
{
    return mBiomeCentreXChunk;
}

int LevelSettings::getBiomeCentreZChunk() const
{
    return mBiomeCentreZChunk;
}

void LevelSettings::validateGameType(int gameType)
{
    GameType::byId(gameType);
}

SuperflatConfig* LevelSettings::getLevelTypeOptions() const
{
    return mLevelTypeOptions;
}
