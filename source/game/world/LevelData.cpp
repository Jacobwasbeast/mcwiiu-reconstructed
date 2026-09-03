#include "game/world/LevelData.hpp"

LevelData::LevelData()
    : mGeneratorType(0), mGeneratorOptions(0), mXSpawn(0), mYSpawn(0), mZSpawn(0),
      mGameTime(0), mDayTime(0), mSpawnXStronghold(0), mSpawnZStronghold(0),
      mSizeOnDisk(0), mVersion(19133), mClearWeatherTime(0), mRaining(false),
      mRainTime(0), mThundering(false), mThunderTime(0), mGameType(0),
      mGenerateMapFeatures(true), mHardcore(false), mAllowCommands(false),
      mInitialized(false), mDifficulty(0), mDifficultyLocked(false), mCloudHeight(128),
      mUseNewSeaLevel(false), mHasBeenInCreative(false), mSpawnBonusChest(false),
      mXZSize(0), mHellScale(8), mModernEnd(false), mBiomeScale(false),
      mBiomeCentreXChunk(0), mBiomeCentreZChunk(0), mLoadedPlayerTag(0),
      mLevelName(), mGameRules()
{
}

LevelData::LevelData(CompoundTag* tag) : LevelData()
{
    if (tag != 0) setTagData(tag);
}

LevelData::LevelData(LevelSettings* settings, const std::wstring& name) : LevelData()
{
    mLevelName = name;
}

LevelData::LevelData(LevelData* other) : LevelData()
{
    if (other != 0) {
        mGeneratorType = other->mGeneratorType; mGeneratorOptions = other->mGeneratorOptions;
        mXSpawn = other->mXSpawn; mYSpawn = other->mYSpawn; mZSpawn = other->mZSpawn;
        mGameTime = other->mGameTime; mDayTime = other->mDayTime;
        mSpawnXStronghold = other->mSpawnXStronghold; mSpawnZStronghold = other->mSpawnZStronghold;
        mSizeOnDisk = other->mSizeOnDisk; mVersion = other->mVersion;
        mClearWeatherTime = other->mClearWeatherTime; mRaining = other->mRaining;
        mRainTime = other->mRainTime; mThundering = other->mThundering; mThunderTime = other->mThunderTime;
        mGameType = other->mGameType; mGenerateMapFeatures = other->mGenerateMapFeatures;
        mHardcore = other->mHardcore; mAllowCommands = other->mAllowCommands; mInitialized = other->mInitialized;
        mDifficulty = other->mDifficulty; mDifficultyLocked = other->mDifficultyLocked;
        mDimensionData = other->mDimensionData; mCloudHeight = other->mCloudHeight;
        mUseNewSeaLevel = other->mUseNewSeaLevel; mHasBeenInCreative = other->mHasBeenInCreative;
        mSpawnBonusChest = other->mSpawnBonusChest; mXZSize = other->mXZSize; mHellScale = other->mHellScale;
        mModernEnd = other->mModernEnd; mBiomeScale = other->mBiomeScale;
        mBiomeCentreXChunk = other->mBiomeCentreXChunk; mBiomeCentreZChunk = other->mBiomeCentreZChunk;
        mLoadedPlayerTag = other->mLoadedPlayerTag; mLevelName = other->mLevelName;
    }
}

LevelData::~LevelData() {}

CompoundTag* LevelData::createTag()
{
    CompoundTag* tag = new CompoundTag();
    tag->putLong(L"RandomSeed", mGameTime);
    tag->putInt(L"SpawnX", mXSpawn); tag->putInt(L"SpawnY", mYSpawn); tag->putInt(L"SpawnZ", mZSpawn);
    tag->putLong(L"Time", mGameTime); tag->putLong(L"LastUpdate", mDayTime);
    tag->putString(L"LevelName", mLevelName);
    return tag;
}

CompoundTag* LevelData::createTag(std::vector<boost::shared_ptr<Player> >* players)
{
    return createTag();
}

void LevelData::addWalkers(DataFixerUpper* fixer)
{
}

void LevelData::setTagData(CompoundTag* tag)
{
    if (tag == 0) return;
    if (tag->contains(L"RandomSeed")) mGameTime = tag->getLong(L"RandomSeed");
    if (tag->contains(L"SpawnX")) mXSpawn = tag->getInt(L"SpawnX");
    if (tag->contains(L"SpawnY")) mYSpawn = tag->getInt(L"SpawnY");
    if (tag->contains(L"SpawnZ")) mZSpawn = tag->getInt(L"SpawnZ");
    if (tag->contains(L"Time")) mGameTime = tag->getLong(L"Time");
    if (tag->contains(L"LastUpdate")) mDayTime = tag->getLong(L"LastUpdate");
    if (tag->contains(L"LevelName")) mLevelName = tag->getString(L"LevelName");
    if (tag->contains(L"raining")) mRaining = tag->getBoolean(L"raining");
    if (tag->contains(L"thundering")) mThundering = tag->getBoolean(L"thundering");
}

long long LevelData::getSeed() { return mGameTime; }
int LevelData::getXSpawn() { return mXSpawn; }
int LevelData::getYSpawn() { return mYSpawn; }
int LevelData::getZSpawn() { return mZSpawn; }
int LevelData::getXStronghold() { return mSpawnXStronghold; }
int LevelData::getZStronghold() { return mSpawnZStronghold; }
int LevelData::getXStrongholdEndPortal() { return mSpawnXStronghold; }
int LevelData::getZStrongholdEndPortal() { return mSpawnZStronghold; }
long long LevelData::getGameTime() { return mGameTime; }
long long LevelData::getDayTime() { return mDayTime; }
long long LevelData::getSizeOnDisk() { return mSizeOnDisk; }
CompoundTag* LevelData::getLoadedPlayerTag() { return mLoadedPlayerTag; }
void LevelData::setSeed(long long value) { mGameTime = value; }
void LevelData::setXSpawn(int value) { mXSpawn = value; }
void LevelData::setYSpawn(int value) { mYSpawn = value; }
void LevelData::setZSpawn(int value) { mZSpawn = value; }
void LevelData::setHasStronghold() { mSpawnXStronghold = 1; }
bool LevelData::getHasStronghold() { return mSpawnXStronghold != 0; }
void LevelData::setXStronghold(int value) { mSpawnXStronghold = value; }
void LevelData::setZStronghold(int value) { mSpawnZStronghold = value; }
void LevelData::setHasStrongholdEndPortal() { mSpawnXStronghold = 1; }
bool LevelData::getHasStrongholdEndPortal() { return mSpawnXStronghold != 0; }
void LevelData::setXStrongholdEndPortal(int value) { mSpawnXStronghold = value; }
void LevelData::setZStrongholdEndPortal(int value) { mSpawnZStronghold = value; }
void LevelData::setGameTime(long long value) { mGameTime = value; }
void LevelData::setDayTime(long long value) { mDayTime = value; }
void LevelData::setSizeOnDisk(long long value) { mSizeOnDisk = value; }
void LevelData::setLoadedPlayerTag(CompoundTag* tag) { mLoadedPlayerTag = tag; }
void LevelData::setSpawn(const BlockPos& pos) { mXSpawn = pos.x; mYSpawn = pos.y; mZSpawn = pos.z; }
std::wstring LevelData::getLevelName() { return mLevelName; }
void LevelData::setLevelName(const std::wstring& name) { mLevelName = name; }
int LevelData::getVersion() { return mVersion; }
void LevelData::setVersion(int value) { mVersion = value; }
long long LevelData::getLastPlayed() { return mDayTime; }
int LevelData::getClearWeatherTime() { return mClearWeatherTime; }
void LevelData::setClearWeatherTime(int value) { mClearWeatherTime = value; }
bool LevelData::isThundering() { return mThundering; }
void LevelData::setThundering(bool value) { mThundering = value; }
int LevelData::getThunderTime() { return mThunderTime; }
void LevelData::setThunderTime(int value) { mThunderTime = value; }
bool LevelData::isRaining() { return mRaining; }
void LevelData::setRaining(bool value) { mRaining = value; }
int LevelData::getRainTime() { return mRainTime; }
void LevelData::setRainTime(int value) { mRainTime = value; }
const GameType* LevelData::getGameType() { return mGameType; }
bool LevelData::isGenerateMapFeatures() { return mGenerateMapFeatures; }
void LevelData::setGenerateMapFeatures(bool value) { mGenerateMapFeatures = value; }
bool LevelData::getSpawnBonusChest() { return mSpawnBonusChest; }
void LevelData::setGameType(const GameType* type) { mGameType = type; mHasBeenInCreative = true; }
bool LevelData::useNewSeaLevel() { return mUseNewSeaLevel; }
bool LevelData::getHasBeenInCreative() { return mHasBeenInCreative; }
void LevelData::setHasBeenInCreative(bool value) { mHasBeenInCreative = value; }
LevelType* LevelData::getGeneratorType() { return mGeneratorType; }
void LevelData::setGeneratorType(LevelType* type) { mGeneratorType = type; }
SuperflatConfig* LevelData::getGeneratorOptions() { return mGeneratorOptions; }
void LevelData::setGeneratorOptions(SuperflatConfig* options) { mGeneratorOptions = options; }
bool LevelData::isHardcore() { return mHardcore; }
void LevelData::setHardcore(bool value) { mHardcore = value; }
bool LevelData::getAllowCommands() { return mAllowCommands; }
void LevelData::setAllowCommands(bool value) { mAllowCommands = value; }
bool LevelData::isInitialized() { return mInitialized; }
void LevelData::setInitialized(bool value) { mInitialized = value; }
GameRules* LevelData::getGameRules() { return &mGameRules; }
const Difficulty* LevelData::getDifficulty() { return mDifficulty; }
void LevelData::setDifficulty(const Difficulty* difficulty) { mDifficulty = difficulty; }
int LevelData::getCloudHeight() { return mCloudHeight; }
void LevelData::setCloudHeight(int value) { mCloudHeight = value; }
bool LevelData::isDifficultyLocked() { return mDifficultyLocked; }
void LevelData::setDifficultyLocked(bool value) { mDifficultyLocked = value; }
CompoundTag* LevelData::getDimensionData(const DimensionType* dimension)
{
    boost::unordered_map<const DimensionType*, CompoundTag*>::iterator it = mDimensionData.find(dimension);
    if (it == mDimensionData.end()) return new CompoundTag();
    return it->second;
}
void LevelData::setDimensionData(const DimensionType* dimension, CompoundTag* tag) { mDimensionData[dimension] = tag; }
int LevelData::getXZSize() { return mXZSize; }
int LevelData::getHellScale() { return mHellScale; }
void LevelData::setModernEnd() { mModernEnd = true; }
bool LevelData::getModernEnd() { return mModernEnd; }
bool LevelData::getBiomeScale() { return mBiomeScale != 0; }
int LevelData::getBiomeCentreXChunk() { return mBiomeCentreXChunk; }
int LevelData::getBiomeCentreZChunk() { return mBiomeCentreZChunk; }
