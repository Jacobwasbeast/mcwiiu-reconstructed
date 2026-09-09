#ifndef GAME_WORLD_GENERATION_PLAYERSTATS_HPP
#define GAME_WORLD_GENERATION_PLAYERSTATS_HPP

#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/core/Scoreboard.hpp"
#include "game/core/StatTracker.hpp"
#include "game/core/StatsUID.hpp"
#include "game/core/KillsWithBowStatTracker.hpp"
#include "game/world/generation/MaxSnowballHitsWithSnowballOnPlayerStatTracker.hpp"

class CheckpointRuleDefinition;
class ContainerInfo;
class DamageEvent;
class DamageSourceMedal;
class GameMedal;
class Item;
class FallKillStatTracker;
class SniperStatTracker;
class TotalKillsStatTracker;

typedef int eTimeSpentCategory;

class PlayerStats
{
public:
    class RoundStats
    {
    public:
        class BlockInteractionInfo
        {
        public:
            BlockInteractionInfo();

            int mCount;
            std::map<int, int> mItems;
            std::map<int, int> mPlayers;
            std::map<int, int> mRounds;
        };

        typedef std::wstring PerLifeStats;

        RoundStats(const StatsUID& statsUID);
        ~RoundStats();

        PerLifeStats* getLifeStats(int life);
        int getNumberLifeStats() const;
        void MedalTimestamp(eTimeSpentCategory category, bool start, long long timestamp);

        std::vector<UntypedStatTracker*> mTrackers;
        bool mWon;
        bool mActive;
        StatTracker<float>* mDamageDealt;
        StatTracker<float>* mDamageReceived;
        UntypedStatTracker* mLastPlayerDamage;
        UntypedStatTracker* mLastPlayerFatalDamage;
        KillsWithBowStatTracker* mKillsWithBow;
        MaxSnowballHitsWithSnowballOnPlayerStatTracker* mMaxSnowballHits;
        int mJumpCount;
        int mKills;
        std::map<int, int> mItems;
        std::vector<ContainerInfo*> mContainers;
        int mWalkDistance;
        int mSprintDistance;
        int mSneakDistance;
        int mSwimDistance;
        int mUnderwaterDistance;
        int mClimbDistance;
        std::vector<Item*> mItemsUsed;
        int mSplashPotionsUsed;
        int mPotionsUsed;
        int mSecondaryTargets;
        int mLifeSpan;
        Scoreboard::Weapon mWeapon;
        int mBlocksPlaced;
        int mItemsCollected;
        int mLayer;
        int mLastCheckpointId;
        std::vector<unsigned int> mCheckpointTimes;
        int mLastCheckpointSequenceId;
        bool mFinished;
        int mLives;
        bool mReachedEnd;
        int mDeaths;
        int mLastPointsAdded;
        int mPoints;
        int mBonusPoints;
        int mRoundTime;
        int mTotalTime;
        double mTopSpeed;
        std::vector<std::wstring> mMedals;
        std::vector<std::wstring> mMedalNames;
        int mDistanceStats[3];
        int mProgressStats[3];
        int mRoundStats[4];
        double mBestTime;
        double mCurrentTime;
        double mPreviousTime;
        boost::unordered_map<int, int> mThermalsEntered;
        int mTimeSpent[6];
        int mTimeSpentPadding;
        long long mTimestamp[6];
        std::set<int> mSpawnedCheckpoints;
        int mCheckpointPadding[8];
        BlockInteractionInfo mBlocksPlacedInfo;
        BlockInteractionInfo mBlocksDestroyed;
        BlockInteractionInfo mBlocksUsed;
        unsigned short mDeathsByType[4];
        unsigned short mKillsByType[4];
        std::wstring mWinningTheme;
        std::vector<PerLifeStats> mLifeStats;
        int mLapsCompleted;
        std::vector<unsigned int> mLayerTime;
        int mLayerTimeCounter;
    };

    PlayerStats(const StatsUID& statsUID);
    ~PlayerStats();

    RoundStats* getRoundStats(unsigned int round);
    void updateTrackedStats(int round, const DamageEvent* damageEvent);
    int getTotalRounds();
    double getDamageDealt(int round);
    double getDamageTaken(int round);
    int getTotalDistanceMoved(unsigned int round);
    double getAllDamageTaken();
    int getTotalJumpCount();
    int getTotalDeaths();
    int getDeaths(unsigned int round);
    int getDeathsInAnyRound(int deaths);
    int getTotalKills();
    void getAllItems(std::map<int, int>& items);
    int getTotalWalkDistance();
    int getTotalSprintDistance();
    int getTotalSneakDistance();
    int getTotalSwimDistance();
    int getTotalUnderwaterDistance();
    int getTotalClimbDistance();
    void getAllItemsUsed(std::vector<Item*>& items);
    int getTotalRoundsWon();
    unsigned char getRoundWon(unsigned int round);
    int getTotalBlocksDestroyed();
    int getTotalSplashPotionsUsed();
    int getTotalPotionsUsed();
    int getTotalLifeSpan();
    int getLifeSpan(unsigned int round);
    int getBestLifeSpan(bool highest, bool reachedEndOnly);
    int getTotalPoints();
    int getPoints(unsigned int round);
    int getBestPoints(bool highest, bool reachedEndOnly);
    unsigned char getReachedEnd(unsigned int round);
    int getTotalThermalsEntered();
    bool hasSpawnedAtCheckpoint(int checkpoint, unsigned int round);
    bool hasSpawnedAtAllCheckpointsInOneRound();
    double getTopSpeed();
    int getImprovement();
    bool hasRoundStats(unsigned int round) const;

    int getTotalSecondaryTargets();
    void getAllContainers(std::vector<ContainerInfo*>& containers);
    int GetLastPointsAdded(unsigned int round);
    int getTotalRoundsReachedEnd();
    bool hasSpawnedAtCheckpoint(int checkpoint);
    bool hasSpawnedAtAllCheckpoints();

private:
    StatsUID mStatsUID;
    std::wstring mName;
    int mCurrentRound;
    std::vector<const GameMedal*> mMedals;
    int mWinningMedal;
    int mLastMedal;

public:
    int mSnowballDirectHits;
    int mHitsFromProjectile;
    StatTracker<float>* mDamageDealt;
    StatTracker<float>* mDamageReceived;
    FallKillStatTracker* mFallKills;
    StatTracker<int>* mAmphibiousKills;
    TotalKillsStatTracker* mTotalKills;
    SniperStatTracker* mSniper;

private:
    friend class DamageSourceMedal;

    UntypedStatTracker* mDamageTakenBySource[15];
    UntypedStatTracker* mWeaponDamage[2];
    UntypedStatTracker* mDeadshot;
    std::vector<UntypedStatTracker*> mTrackers;
    boost::unordered_map<int, RoundStats*> mRoundStats;
};

#endif
