#ifndef GAME_WORLD_GENERATION_PLAYERSTATS_HPP
#define GAME_WORLD_GENERATION_PLAYERSTATS_HPP
#include <vector>
#include <map>
#include <set>
#include <string>
#include <boost/unordered_map.hpp>
#include "game/core/StatsUID.hpp"
#include "game/core/UntypedStatTracker.hpp"
class DamageEvent;
class Item;
class ContainerInfo;
typedef int eTimeSpentCategory;
namespace Scoreboard { class Weapon { public: Weapon(); int mItemId; int mPotionId; }; }
class PlayerStats
{
public:
 class RoundStats
 {
 public:
  class BlockInteractionInfo { public: BlockInteractionInfo(); int mCount; std::map<int,int> placed; std::map<int,int> mined; std::map<int,int> used; };
  struct PerLifeStats { std::vector<UntypedStatTracker*> trackers; int values[5]; PerLifeStats(); };
  RoundStats(const StatsUID&); ~RoundStats();
  std::vector<UntypedStatTracker*> trackers; bool won; bool reachedEnd; int round; int deaths; int kills; int points; int jumpCount; int distance[2]; int mDistance[6]; UntypedStatTracker* mDamageDealt; std::vector<ContainerInfo*> containers; std::vector<Item*> itemsUsedList; Scoreboard::Weapon weapon; int itemsUsed; int potions; std::vector<unsigned int> checkpointTimes; int splashPotions; bool reachedEndFlag; int thermals; bool secondaryTargetsFlag; int secondaryTargets; int lastPointsAdded; int mTimeSpent[2]; double topSpeed; std::vector<std::wstring> medals; std::vector<std::wstring> medalNames; double mBestTime; double mCurrentTime; double mTotalTime; boost::unordered_map<int,int> itemCounts; BlockInteractionInfo blocksDestroyed; BlockInteractionInfo blocksPlaced; BlockInteractionInfo blocksUsed; int mReserved[4]; std::vector<PerLifeStats> lifeStats;
  PerLifeStats* getLifeStats(int); int getNumberLifeStats() const; void MedalTimestamp(eTimeSpentCategory,bool,long long);
 };
 PlayerStats(const StatsUID&); ~PlayerStats();
 RoundStats* getRoundStats(unsigned int); void updateTrackedStats(int,const DamageEvent*); int getTotalRounds(); double getDamageDealt(int); double getDamageTaken(int); int getTotalDistanceMoved(unsigned int); double getAllDamageTaken(); int getTotalJumpCount(); int getTotalDeaths(); int getDeaths(unsigned int); int getDeathsInAnyRound(int); int getTotalKills(); void getAllItems(std::map<int,int>&) const; int getTotalWalkDistance(); int getTotalSprintDistance(); int getTotalSneakDistance(); int getTotalSwimDistance(); int getTotalUnderwaterDistance(); int getTotalClimbDistance(); void getAllItemsUsed(std::vector<Item*>&) const; int getTotalRoundsWon(); unsigned char getRoundWon(unsigned int); int getTotalBlocksDestroyed(); int getTotalSplashPotionsUsed(); int getTotalPotionsUsed(); int getTotalLifeSpan(); int getLifeSpan(unsigned int); int getBestLifeSpan(bool,bool); int getTotalPoints(); int getPoints(unsigned int); int getBestPoints(bool,bool); unsigned char getReachedEnd(unsigned int); int getTotalThermalsEntered(); bool hasSpawnedAtCheckpoint(int,unsigned int); bool hasSpawnedAtAllCheckpointsInOneRound(); double getTopSpeed(); int getImprovement(); bool hasRoundStats(unsigned int) const; int getTotalSecondaryTargets(); void getAllContainers(std::vector<ContainerInfo*>&) const; int GetLastPointsAdded(unsigned int); int getTotalRoundsReachedEnd(); bool hasSpawnedAtCheckpoint(int); bool hasSpawnedAtAllCheckpoints();
private:
 StatsUID mStatsUID; std::vector<const void*> mMedals; std::vector<UntypedStatTracker*> mTrackers; std::map<int,int> mItems; boost::unordered_map<int,RoundStats*> mRoundStats;
};
#endif
