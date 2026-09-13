#ifndef GAME_WORLD_LEVELSETTINGS_HPP
#define GAME_WORLD_LEVELSETTINGS_HPP
#include "game/core/GameType.hpp"
#include "game/world/LevelType.hpp"
class LevelData; class SuperflatConfig;
class LevelSettings { public:
 LevelSettings(long long,const GameType*,bool,bool,bool,LevelType*,int,int,int,bool,unsigned char,int,int);
 explicit LevelSettings(LevelData*);
 void _init(long long,const GameType*,bool,bool,bool,LevelType*,int,int,int,bool,unsigned char,int,int);
 void enableStartingBonusItems(); void enableSinglePlayerCommands(); void setLevelTypeOptions(SuperflatConfig*);
 bool hasStartingBonusItems() const; long long getSeed() const; const GameType* getGameType() const; bool isHardcore() const;
 LevelType* getLevelType() const; bool getAllowCommands() const; bool isGenerateMapFeatures() const; int getCloudHeight() const;
 bool useNewSeaLevel() const; int getXZSize() const; int getHellScale() const; bool getModernEnd() const;
 unsigned char getBiomeScale() const; int getBiomeCentreXChunk() const; int getBiomeCentreZChunk() const;
 static const GameType* validateGameType(int); SuperflatConfig* getLevelTypeOptions() const;
 private: long long mSeed; const GameType* mGameType; bool mGenerateMapFeatures; bool mHardcore; int mCloudHeight;
 bool mUseNewSeaLevel; LevelType* mLevelType; bool mAllowCommands; bool mStartingBonusItems; SuperflatConfig* mLevelTypeOptions;
 int mXZSize; int mHellScale; bool mModernEnd; unsigned char mBiomeScale; int mBiomeCentreXChunk; int mBiomeCentreZChunk;
};
#endif
