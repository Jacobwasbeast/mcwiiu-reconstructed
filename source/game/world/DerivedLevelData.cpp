#include "game/world/DerivedLevelData.hpp"

DerivedLevelData::DerivedLevelData(LevelData* levelData) : LevelData(), mLevelData(levelData)
{
}

DerivedLevelData::~DerivedLevelData()
{
}

CompoundTag* DerivedLevelData::createTag(std::vector<boost::shared_ptr<Player> >* players)
{
    return mLevelData->createTag(players);
}

void DerivedLevelData::setTagData(CompoundTag* tag) { mLevelData->setTagData(tag); }
long long DerivedLevelData::getSeed() { return mLevelData->getSeed(); }
int DerivedLevelData::getXSpawn() { return mLevelData->getXSpawn(); }
int DerivedLevelData::getYSpawn() { return mLevelData->getYSpawn(); }
int DerivedLevelData::getZSpawn() { return mLevelData->getZSpawn(); }
long long DerivedLevelData::getGameTime() { return mLevelData->getGameTime(); }
long long DerivedLevelData::getDayTime() { return mLevelData->getDayTime(); }
long long DerivedLevelData::getSizeOnDisk() { return mLevelData->getSizeOnDisk(); }
CompoundTag* DerivedLevelData::getLoadedPlayerTag() { return mLevelData->getLoadedPlayerTag(); }
void DerivedLevelData::setSeed(long long value) { mLevelData->setSeed(value); }
void DerivedLevelData::setXSpawn(int value) { mLevelData->setXSpawn(value); }
void DerivedLevelData::setYSpawn(int value) { mLevelData->setYSpawn(value); }
void DerivedLevelData::setZSpawn(int value) { mLevelData->setZSpawn(value); }
void DerivedLevelData::setGameTime(long long value) { mLevelData->setGameTime(value); }
void DerivedLevelData::setDayTime(long long value) { mLevelData->setDayTime(value); }
void DerivedLevelData::setSizeOnDisk(long long value) { mLevelData->setSizeOnDisk(value); }
void DerivedLevelData::setLoadedPlayerTag(CompoundTag* tag) { mLevelData->setLoadedPlayerTag(tag); }
void DerivedLevelData::setSpawn(const BlockPos& pos) { mLevelData->setSpawn(pos); }
std::wstring DerivedLevelData::getLevelName() { return mLevelData->getLevelName(); }
void DerivedLevelData::setLevelName(const std::wstring& name) { mLevelData->setLevelName(name); }
int DerivedLevelData::getVersion() { return mLevelData->getVersion(); }
void DerivedLevelData::setVersion(int value) { mLevelData->setVersion(value); }
long long DerivedLevelData::getLastPlayed() { return mLevelData->getLastPlayed(); }
bool DerivedLevelData::isThundering() { return mLevelData->isThundering(); }
int DerivedLevelData::getThunderTime() { return mLevelData->getThunderTime(); }
bool DerivedLevelData::isRaining() { return mLevelData->isRaining(); }
int DerivedLevelData::getRainTime() { return mLevelData->getRainTime(); }
const GameType* DerivedLevelData::getGameType() { return mLevelData->getGameType(); }
void DerivedLevelData::setThundering(bool value) { mLevelData->setThundering(value); }
void DerivedLevelData::setThunderTime(int value) { mLevelData->setThunderTime(value); }
void DerivedLevelData::setRaining(bool value) { mLevelData->setRaining(value); }
void DerivedLevelData::setRainTime(int value) { mLevelData->setRainTime(value); }
bool DerivedLevelData::isGenerateMapFeatures() { return mLevelData->isGenerateMapFeatures(); }
void DerivedLevelData::setGameType(const GameType* type) { mLevelData->setGameType(type); }
bool DerivedLevelData::isHardcore() { return mLevelData->isHardcore(); }
LevelType* DerivedLevelData::getGeneratorType() { return mLevelData->getGeneratorType(); }
void DerivedLevelData::setGeneratorType(LevelType* type) { mLevelData->setGeneratorType(type); }
bool DerivedLevelData::getAllowCommands() { return mLevelData->getAllowCommands(); }
void DerivedLevelData::setAllowCommands(bool value) { mLevelData->setAllowCommands(value); }
bool DerivedLevelData::isInitialized() { return mLevelData->isInitialized(); }
void DerivedLevelData::setInitialized(bool value) { mLevelData->setInitialized(value); }
GameRules* DerivedLevelData::getGameRules() { return mLevelData->getGameRules(); }
const Difficulty* DerivedLevelData::getDifficulty() { return mLevelData->getDifficulty(); }
void DerivedLevelData::setDifficulty(const Difficulty* difficulty) { mLevelData->setDifficulty(difficulty); }
bool DerivedLevelData::isDifficultyLocked() { return mLevelData->isDifficultyLocked(); }
void DerivedLevelData::setDifficultyLocked(bool value) { mLevelData->setDifficultyLocked(value); }
void DerivedLevelData::setDimensionData(const DimensionType* dimension, CompoundTag* tag) { mLevelData->setDimensionData(dimension, tag); }
CompoundTag* DerivedLevelData::getDimensionData(const DimensionType* dimension) { return mLevelData->getDimensionData(dimension); }
int DerivedLevelData::getXZSize() { return mLevelData->getXZSize(); }
int DerivedLevelData::getHellScale() { return mLevelData->getHellScale(); }
void DerivedLevelData::setModernEnd() { mLevelData->setModernEnd(); }
bool DerivedLevelData::getModernEnd() { return mLevelData->getModernEnd(); }
