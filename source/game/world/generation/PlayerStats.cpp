#include "game/world/generation/PlayerStats.hpp"
#include <algorithm>
#include <climits>
#include <cmath>

#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/System.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/DamageDealtStatTracker.hpp"
#include "game/core/DamageReceivedStatTracker.hpp"
#include "game/world/generation/LastPlayerDamageStatTracker.hpp"
#include "game/world/generation/LastPlayerFatalDamageStatTracker.hpp"
#include "game/core/FallKillStatTracker.hpp"
#include "game/core/AmphibiousKillStatTracker.hpp"
#include "game/core/TotalKillsStatTracker.hpp"
#include "game/core/SniperStatTracker.hpp"
#include "game/core/DamageTakenBySourceStatTracker.hpp"
#include "game/core/WeaponDamageStatTracker.hpp"
#include "game/core/DeadshotStatTracker.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/BlockPos.hpp"

PlayerStats::RoundStats::BlockInteractionInfo::BlockInteractionInfo()
    : mCount(0)
    , mItems()
    , mPlayers()
    , mRounds()
{
}

PlayerStats::PlayerStats(const StatsUID& statsUID)
    : mStatsUID(statsUID)
    , mName()
    , mCurrentRound(0)
    , mMedals()
    , mWinningMedal(-1)
    , mLastMedal(0)
    , mSnowballDirectHits(0)
    , mHitsFromProjectile(0)
    , mDamageDealt(new DamageDealtStatTracker(mStatsUID))
    , mDamageReceived(new DamageReceivedStatTracker(mStatsUID))
    , mFallKills(new FallKillStatTracker(mStatsUID))
    , mAmphibiousKills(new AmphibiousKillStatTracker(mStatsUID))
    , mTotalKills(new TotalKillsStatTracker(mStatsUID))
    , mSniper(new SniperStatTracker(mStatsUID))
    , mDamageTakenBySource()
    , mWeaponDamage()
    , mDeadshot(0)
    , mTrackers()
    , mRoundStats()
{
    mTrackers.push_back(mDamageDealt);
    mTrackers.push_back(mDamageReceived);
    mTrackers.push_back(mFallKills);
    mTrackers.push_back(mAmphibiousKills);
    mTrackers.push_back(mTotalKills);
    mTrackers.push_back(mSniper);

    for (int source = 0; source < 15; ++source)
    {
        mDamageTakenBySource[source] = new DamageTakenBySourceStatTracker(
            mStatsUID, static_cast<Scoreboard::EDamageSource>(source));
        mTrackers.push_back(mDamageTakenBySource[source]);
    }

    mWeaponDamage[0] = new WeaponDamageStatTracker(mStatsUID, WeaponDamageStatTracker::TRACKING_DEALT);
    mWeaponDamage[1] = new WeaponDamageStatTracker(mStatsUID, WeaponDamageStatTracker::TRACKING_RECEIVED);
    mTrackers.push_back(mWeaponDamage[0]);
    mTrackers.push_back(mWeaponDamage[1]);

    mDeadshot = new DeadshotStatTracker(mStatsUID);
    mTrackers.push_back(mDeadshot);
}

PlayerStats::~PlayerStats()
{
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        delete round->second;
    }

    std::vector<UntypedStatTracker*>::iterator tracker = mTrackers.begin();
    for (; tracker != mTrackers.end(); ++tracker)
    {
        delete *tracker;
    }
}

PlayerStats::RoundStats::RoundStats(const StatsUID& statsUID)
    : mTrackers()
    , mWon(false)
    , mActive(false)
    , mDamageDealt(new DamageDealtStatTracker(statsUID))
    , mDamageReceived(new DamageReceivedStatTracker(statsUID))
    , mLastPlayerDamage(new LastPlayerDamageStatTracker(statsUID))
    , mLastPlayerFatalDamage(new LastPlayerFatalDamageStatTracker(statsUID))
    , mKillsWithBow(new KillsWithBowStatTracker(statsUID))
    , mMaxSnowballHits(new MaxSnowballHitsWithSnowballOnPlayerStatTracker(statsUID))
    , mJumpCount(0)
    , mKills(0)
    , mItems()
    , mContainers()
    , mWalkDistance(0)
    , mSprintDistance(0)
    , mSneakDistance(0)
    , mSwimDistance(0)
    , mUnderwaterDistance(0)
    , mClimbDistance(0)
    , mItemsUsed()
    , mSplashPotionsUsed(0)
    , mPotionsUsed(0)
    , mSecondaryTargets(0)
    , mLifeSpan(0)
    , mWeapon()
    , mBlocksPlaced(0)
    , mItemsCollected(0)
    , mLayer(-1)
    , mLastCheckpointId(0)
    , mCheckpointTimes()
    , mLastCheckpointSequenceId(-1)
    , mFinished(false)
    , mLives(0)
    , mReachedEnd(false)
    , mDeaths(0)
    , mLastPointsAdded(0)
    , mPoints(0)
    , mBonusPoints(0)
    , mRoundTime(0)
    , mTotalTime(0)
    , mTopSpeed(0.0)
    , mMedals()
    , mMedalNames()
    , mDistanceStats()
    , mProgressStats()
    , mRoundStats()
    , mBestTime(0.0)
    , mCurrentTime(0.0)
    , mPreviousTime(0.0)
    , mThermalsEntered()
    , mTimeSpent()
    , mTimeSpentPadding(0)
    , mTimestamp()
    , mSpawnedCheckpoints()
    , mCheckpointPadding()
    , mBlocksPlacedInfo()
    , mBlocksDestroyed()
    , mBlocksUsed()
    , mDeathsByType()
    , mKillsByType()
    , mWinningTheme()
    , mLifeStats()
    , mLapsCompleted(0)
    , mLayerTime()
    , mLayerTimeCounter(0)
{
    mTrackers.push_back(mDamageDealt);
    mTrackers.push_back(mDamageReceived);
    mTrackers.push_back(mLastPlayerDamage);
    mTrackers.push_back(mLastPlayerFatalDamage);
    mTrackers.push_back(mKillsWithBow);
    mTrackers.push_back(mMaxSnowballHits);
}

PlayerStats::RoundStats::~RoundStats()
{
    std::vector<UntypedStatTracker*>::iterator tracker = mTrackers.begin();
    for (; tracker != mTrackers.end(); ++tracker)
    {
        delete *tracker;
    }
}

PlayerStats::RoundStats* PlayerStats::getRoundStats(unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator found = mRoundStats.find(round);
    if (found != mRoundStats.end())
    {
        return found->second;
    }

    RoundStats* stats = new RoundStats(mStatsUID);
    mRoundStats[round] = stats;
    return stats;
}

void PlayerStats::updateTrackedStats(int round, const DamageEvent* damageEvent)
{
    std::vector<UntypedStatTracker*>::iterator tracker = mTrackers.begin();
    for (; tracker != mTrackers.end(); ++tracker)
    {
        (*tracker)->update(damageEvent);
    }

    RoundStats* stats = getRoundStats(round);
    tracker = stats->mTrackers.begin();
    for (; tracker != stats->mTrackers.end(); ++tracker)
    {
        (*tracker)->update(damageEvent);
    }
}

int PlayerStats::getTotalRounds()
{
    if (mRoundStats.empty())
    {
        return 1;
    }

    unsigned int highestRound = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        if (highestRound < static_cast<unsigned int>(round->first))
        {
            highestRound = round->first;
        }
    }
    return highestRound + 1;
}

double PlayerStats::getDamageDealt(int round)
{
    return getRoundStats(round)->mDamageDealt->getValue();
}

double PlayerStats::getDamageTaken(int round)
{
    return getRoundStats(round)->mDamageReceived->getValue();
}

int PlayerStats::getTotalDistanceMoved(unsigned int round)
{
    RoundStats* stats = getRoundStats(round);
    return stats->mWalkDistance + stats->mSprintDistance + stats->mSneakDistance
        + stats->mSwimDistance + stats->mUnderwaterDistance + stats->mClimbDistance;
}

double PlayerStats::getAllDamageTaken()
{
    return mDamageReceived->getValue();
}

int PlayerStats::getTotalJumpCount()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mJumpCount;
    }
    return total;
}

int PlayerStats::getTotalDeaths()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mDeaths;
    }
    return total;
}

int PlayerStats::getDeaths(unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator found = mRoundStats.find(round);
    return found == mRoundStats.end() ? 0 : found->second->mDeaths;
}

int PlayerStats::getDeathsInAnyRound(int deaths)
{
    int result = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        int roundDeaths = round->second->mDeaths;
        if (Mth::abs(roundDeaths - deaths) < INT_MAX)
        {
            result = roundDeaths;
        }
    }
    return result;
}

int PlayerStats::getTotalKills()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mKills;
    }
    return total;
}

void PlayerStats::getAllItems(std::map<int, int>& items)
{
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        items.insert(round->second->mItems.begin(), round->second->mItems.end());
    }
}

int PlayerStats::getTotalWalkDistance()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mWalkDistance;
    }
    return total;
}

int PlayerStats::getTotalSprintDistance()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mSprintDistance;
    }
    return total;
}

int PlayerStats::getTotalSneakDistance()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mSneakDistance;
    }
    return total;
}

int PlayerStats::getTotalSwimDistance()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mSwimDistance;
    }
    return total;
}

int PlayerStats::getTotalUnderwaterDistance()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mUnderwaterDistance;
    }
    return total;
}

int PlayerStats::getTotalClimbDistance()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mClimbDistance;
    }
    return total;
}

void PlayerStats::getAllItemsUsed(std::vector<Item*>& items)
{
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        items.insert(items.end(), round->second->mItemsUsed.begin(), round->second->mItemsUsed.end());
    }
}

int PlayerStats::getTotalRoundsWon()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        if (round->second->mWon)
        {
            ++total;
        }
    }
    return total;
}

unsigned char PlayerStats::getRoundWon(unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator found = mRoundStats.find(round);
    return found == mRoundStats.end() ? 0 : found->second->mWon;
}

int PlayerStats::getTotalBlocksDestroyed()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mBlocksDestroyed.mCount;
    }
    return total;
}

int PlayerStats::getTotalSplashPotionsUsed()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mSplashPotionsUsed;
    }
    return total;
}

int PlayerStats::getTotalPotionsUsed()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mPotionsUsed;
    }
    return total;
}

int PlayerStats::getTotalLifeSpan()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mLifeSpan;
    }
    return total;
}

int PlayerStats::getLifeSpan(unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator found = mRoundStats.find(round);
    return found == mRoundStats.end() ? 0 : found->second->mLifeSpan;
}

int PlayerStats::getBestLifeSpan(bool highest, bool reachedEndOnly)
{
    int best = highest ? 0 : -1;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        RoundStats* stats = round->second;
        if (reachedEndOnly && !stats->mReachedEnd)
        {
            continue;
        }

        if (highest ? best < stats->mLifeSpan : static_cast<unsigned int>(stats->mLifeSpan) < static_cast<unsigned int>(best))
        {
            best = stats->mLifeSpan;
        }
    }
    return best;
}

int PlayerStats::getTotalPoints()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mPoints + round->second->mBonusPoints;
    }
    return total;
}

int PlayerStats::getPoints(unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator found = mRoundStats.find(round);
    if (found == mRoundStats.end())
    {
        return 0;
    }
    return found->second->mPoints + found->second->mBonusPoints;
}

int PlayerStats::getBestPoints(bool highest, bool reachedEndOnly)
{
    int best = highest ? 0 : -1;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        RoundStats* stats = round->second;
        if (reachedEndOnly && !stats->mReachedEnd)
        {
            continue;
        }

        int points = stats->mPoints + stats->mBonusPoints;
        if (highest ? best < points : static_cast<unsigned int>(points) < static_cast<unsigned int>(best))
        {
            best = points;
        }
    }
    return best;
}

unsigned char PlayerStats::getReachedEnd(unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator found = mRoundStats.find(round);
    return found == mRoundStats.end() ? 0 : found->second->mReachedEnd;
}

int PlayerStats::getTotalThermalsEntered()
{
    int total = 0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        total += round->second->mThermalsEntered.size();
    }
    return total;
}

bool PlayerStats::hasSpawnedAtCheckpoint(int checkpoint, unsigned int round)
{
    boost::unordered_map<int, RoundStats*>::iterator found = mRoundStats.find(round);
    if (found == mRoundStats.end())
    {
        return false;
    }
    return found->second->mSpawnedCheckpoints.find(checkpoint) != found->second->mSpawnedCheckpoints.end();
}

bool PlayerStats::hasSpawnedAtAllCheckpointsInOneRound()
{
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options == 0)
    {
        return false;
    }

    LevelRuleset* ruleset = options->getRequiredGameRules();
    if (ruleset == 0)
    {
        return false;
    }

    std::vector<CheckpointRuleDefinition*>* checkpoints = ruleset->getCheckpoints();
    for (unsigned int round = 0; round < static_cast<unsigned int>(getTotalRounds()); ++round)
    {
        bool allCheckpoints = true;
        std::vector<CheckpointRuleDefinition*>::iterator checkpoint = checkpoints->begin();
        for (; checkpoint != checkpoints->end(); ++checkpoint)
        {
            if ((*checkpoint)->hasSpawnPoint()
                && !hasSpawnedAtCheckpoint((*checkpoint)->getSequenceID(), round))
            {
                allCheckpoints = false;
                break;
            }
        }

        if (allCheckpoints)
        {
            return true;
        }
    }
    return false;
}

double PlayerStats::getTopSpeed()
{
    double topSpeed = 0.0;
    boost::unordered_map<int, RoundStats*>::iterator round = mRoundStats.begin();
    for (; round != mRoundStats.end(); ++round)
    {
        if (topSpeed < round->second->mTopSpeed)
        {
            topSpeed = round->second->mTopSpeed;
        }
    }
    return topSpeed;
}

int PlayerStats::getImprovement()
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    int lastRound = miniGame->GetFinalRoundIndex();
    int firstRound = 0;

    while (firstRound <= lastRound && !hasRoundStats(firstRound))
    {
        ++firstRound;
    }
    while (firstRound <= lastRound && !hasRoundStats(lastRound))
    {
        --lastRound;
    }

    if (firstRound == lastRound)
    {
        return 0;
    }
    if (miniGame->GetGameVariant() == 0)
    {
        return getLifeSpan(firstRound) - getLifeSpan(lastRound);
    }
    if (miniGame->GetGameVariant() == 1)
    {
        return getPoints(lastRound) - getPoints(firstRound);
    }
    return 0;
}

bool PlayerStats::hasRoundStats(unsigned int round) const
{
    return mRoundStats.find(round) != mRoundStats.end();
}

PlayerStats::RoundStats::PerLifeStats* PlayerStats::RoundStats::getLifeStats(int life)
{
    if (life >= static_cast<int>(mLifeStats.size()))
    {
        PerLifeStats stats;
        mLifeStats.push_back(stats);
    }
    return &mLifeStats.at(life);
}

int PlayerStats::RoundStats::getNumberLifeStats() const
{
    return mLifeStats.size();
}

void PlayerStats::RoundStats::MedalTimestamp(
    eTimeSpentCategory category, bool start, long long timestamp)
{
    if (start)
    {
        if (mTimestamp[category] == 0)
        {
            mTimestamp[category] = timestamp;
        }
        return;
    }

    if (mTimestamp[category] != 0)
    {
        mTimeSpent[category] += static_cast<int>(timestamp - mTimestamp[category]);
        mTimestamp[category] = 0;
    }
}

int getBlocksPlaced(const PlayerStats::RoundStats& roundStats)
{
    return roundStats.mBlocksPlacedInfo.mCount;
}

int getBlocksDestroyed(const PlayerStats::RoundStats& roundStats)
{
    return roundStats.mBlocksDestroyed.mCount;
}

int getBlocksPresent(const PlayerStats::RoundStats& roundStats)
{
    return roundStats.mBlocksUsed.mCount;
}

int wasWinnerWithoutDestroyingBlocks(const PlayerStats::RoundStats& roundStats)
{
    return roundStats.mWon && roundStats.mBlocksDestroyed.mCount == 0;
}

void writeBlockInteractionData(PlayerStats::RoundStats& roundStats,
                               PlayerStats::RoundStats::BlockInteractionInfo& info,
                               const long long& timestamp,
                               const BlockPos& pos,
                               int itemId)
{
    ++info.mCount;
    ++info.mItems[itemId];
    ++info.mPlayers[pos.y - roundStats.mLayer];

    const double elapsed = std::floor(
        static_cast<double>(System::processTimeInMilliSecs() - timestamp));
    const double timeBucket = elapsed / 10000.0;
    ++info.mRounds[static_cast<int>(timeBucket)];
}
