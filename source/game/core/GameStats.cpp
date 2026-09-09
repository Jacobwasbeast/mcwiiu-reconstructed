#include "game/core/GameStats.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/CombatEntry.hpp"
#include "game/core/System.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Math.hpp"
#include "game/server/ServerPlayer.hpp"

GameStats::GameStats()
    : mPlayerCount(0), mRoundStartTime(0), mCurrentRound(0), mRoundEndTime(0), mRoundTime(0), mRoundWinner(), mFirstPlace(), mSecondPlace(), mThirdPlace(), mFarthestCheckpointReached(0), mLandingRecorded(false)
{
}

GameStats::~GameStats()
{
    mPlayerStats.clear();
}

PlayerStats* GameStats::GetPlayerStats(const StatsUID& uid)
{
    StatsMap::iterator it = mPlayerStats.find(uid);
    return it == mPlayerStats.end() ? 0 : it->second;
}

boost::shared_ptr<ServerPlayer> GameStats::GetFirstPlace()
{
    return mFirstPlace;
}

unsigned int GameStats::GetShortestLifespan(unsigned int) const
{
    return 0x7fffffffU;
}

GameStats::StatsMap* GameStats::GetPlayerStats()
{
    return &mPlayerStats;
}

unsigned int GameStats::GetDeathCount(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return 0;
    return static_cast<unsigned int>(stats->getRoundStats(mCurrentRound)->mDeaths);
}

void GameStats::RecordBlockPresent(const StatsUID& uid, const BlockPos& pos, int state)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;

    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    PlayerStats::RoundStats::BlockInteractionInfo& info = round->mBlocksUsed;
    ++info.mCount;
    ++info.mItems[state];
    ++info.mPlayers[pos.y - round->mLayer];
    const double elapsed = static_cast<double>(System::processTimeInMilliSecs() - mRoundStartTime);
    ++info.mRounds[static_cast<int>(elapsed / 10000.0)];
}

unsigned int GameStats::GetRoundTime()
{
    if (mRoundStartTime == 0)
        return 0;
    unsigned long long now = static_cast<unsigned long long>(System::processTimeInMilliSecs());
    return static_cast<unsigned int>((now - mRoundStartTime) / 1000);
}

void GameStats::AwardMG01Medals(const std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    if (players == 0)
        return;

    PlayerStats* best = 0;
    bool tied = false;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
         it != players->end(); ++it) {
        if (!*it)
            continue;
        PlayerStats* stats = GetPlayerStats((*it)->getSUID());
        if (stats == 0 || !stats->hasRoundStats(mCurrentRound))
            continue;
        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        if (round == 0)
            continue;
        round->mActive = true;
        if (best == 0 || round->mKills > best->getRoundStats(mCurrentRound)->mKills) {
            best = stats;
            tied = false;
        } else if (round->mKills == best->getRoundStats(mCurrentRound)->mKills) {
            tied = true;
        }
    }
    if (best != 0 && !tied)
        best->getRoundStats(mCurrentRound)->mWon = true;
}
bool GameStats::SORT_MG02_STATS(PlayerStats* first, PlayerStats* second) { return first < second; }
PlayerStats* GameStats::getPlayerFromStatsVec(const std::vector<PlayerStats*>& stats, unsigned int index) { return index < stats.size() ? stats[index] : 0; }
void GameStats::AwardMG02Medals(const std::vector<boost::shared_ptr<ServerPlayer> >*)
{
}
bool GameStats::SORT_MG03_STATS_FALLBACK(PlayerStats* first, PlayerStats* second) { return first < second; }
bool GameStats::SORT_MG03_STATS_POINTS(PlayerStats* first, PlayerStats* second) { return first < second; }
bool GameStats::SORT_MG03_STATS_TIME(PlayerStats* first, PlayerStats* second) { return first < second; }
bool GameStats::SORT_MG03_STATS_POINTS_CURRENT_ROUND(PlayerStats* first, PlayerStats* second) { return first < second; }
bool GameStats::SORT_MG03_STATS_TIME_CURRENT_ROUND(PlayerStats* first, PlayerStats* second) { return first < second; }
void GameStats::AwardMG03Medals(const std::vector<boost::shared_ptr<ServerPlayer> >*)
{
}
void GameStats::AwardMG04Medals(const std::vector<boost::shared_ptr<ServerPlayer> >*)
{
    mRoundWinner.reset();
    mFirstPlace.reset();
    mSecondPlace.reset();
    mThirdPlace.reset();
}
void GameStats::CalculateStats(const std::vector<boost::shared_ptr<ServerPlayer> >*, bool)
{
}

void GameStats::AddPlayerStats(const boost::shared_ptr<ServerPlayer>&) { ++mPlayerCount; }
void GameStats::AddPlayers(const std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    if (players != 0)
        for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin(); it != players->end(); ++it)
            AddPlayerStats(*it);
}
void GameStats::RemovePlayerStats(const boost::shared_ptr<ServerPlayer>&) { if (mPlayerCount != 0) --mPlayerCount; }
void GameStats::Clear() { mPlayerStats.clear(); mPlayerCount = 0; mRoundTime = 0; mFarthestCheckpointReached = 0; mLandingRecorded = false; }
void GameStats::OnRoundStart() { mRoundStartTime = static_cast<unsigned int>(System::processTimeInMilliSecs()); mLandingRecorded = false; }
void GameStats::OnRoundEnd() { mRoundEndTime = static_cast<unsigned int>(System::processTimeInMilliSecs()); }
void GameStats::OnGameStart() { Clear(); OnRoundStart(); }
void GameStats::OnGameEnd() { mRoundEndTime = static_cast<unsigned int>(System::processTimeInMilliSecs()); }

int GameStats::GetDamageTakenThisRound(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0 : static_cast<int>(stats->getRoundStats(mCurrentRound)->mDamageReceived->getValue());
}
unsigned int GameStats::GetKillsWithBowThisRound(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0 : static_cast<unsigned int>(stats->getRoundStats(mCurrentRound)->mKillsWithBow->getValue());
}
unsigned int GameStats::GetMaxSnowballHitsOnAnyPlayer(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0 : stats->getRoundStats(mCurrentRound)->mMaxSnowballHits->getValue();
}
void GameStats::RecordDamage(boost::shared_ptr<Entity>, CombatEntry*, boost::shared_ptr<ClientboundChatPacket>) { }
unsigned int GameStats::GetPlayerFatalDamage(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0U : static_cast<unsigned int>(stats->getRoundStats(mCurrentRound)->mDamageReceived->getValue());
}
void GameStats::RecordDeath(boost::shared_ptr<Entity>) { }
void GameStats::RecordJump(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) ++stats->getRoundStats(mCurrentRound)->mJumpCount;
}
void GameStats::RecordItem(const StatsUID& uid, not_null_ptr<ItemInstance> item)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;

    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    ItemInstance* instance = item.get();
    if (instance == NULL)
        return;

    Item* itemType = instance->getItem();
    const int itemId = Item::VALIDgetId(itemType);
    std::map<int, int>::iterator it = round->mItems.find(itemId);
    if (it != round->mItems.end())
        it->second += instance->getCount();
    else
        round->mItems.insert(std::make_pair(itemId, instance->getCount()));
}
void GameStats::RecordContainer(const StatsUID&, int, int, int, int) { }
void GameStats::RecordWalk(const StatsUID& uid, int distance, bool sprinting, bool sneaking) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL) return;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    if (sprinting) round->mSprintDistance += distance;
    else if (sneaking) round->mSneakDistance += distance;
    else round->mWalkDistance += distance;
}
void GameStats::RecordSwim(const StatsUID& uid, int distance, bool underwater) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL) return;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    if (underwater) round->mUnderwaterDistance += distance;
    else round->mSwimDistance += distance;
}
void GameStats::RecordClimb(const StatsUID& uid, int distance) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) stats->getRoundStats(mCurrentRound)->mClimbDistance += distance;
}
void GameStats::RecordGainPoints(const StatsUID& uid, int points) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) stats->getRoundStats(mCurrentRound)->mBonusPoints += points;
}
void GameStats::ResetTempPoints(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    round->mLastPointsAdded = 0;
    round->mBonusPoints = 0;
    round->mBlocksUsed = PlayerStats::RoundStats::BlockInteractionInfo();
}
unsigned int GameStats::RecordSecondaryTargetReached(const StatsUID& uid, const std::wstring& target, int points,
                                                     GlideRingGenerator::eGlideRingSize size)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return 0;

    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    std::vector<std::wstring>::const_iterator it;
    for (it = round->mMedals.begin(); it != round->mMedals.end(); ++it)
        if (*it == target)
            return 0;
    for (it = round->mMedalNames.begin(); it != round->mMedalNames.end(); ++it)
        if (*it == target)
            return 0;

    ++round->mSecondaryTargets;
    round->mMedals.push_back(target);
    round->mTotalTime += points;
    round->mTimeSpent[static_cast<int>(size)]++;
    return 1;
}
void GameStats::TakeTimestamp(const StatsUID& uid, eTimeSpentCategory category, bool start) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) stats->getRoundStats(mCurrentRound)->MedalTimestamp(category, start, System::processTimeInMilliSecs());
}
void GameStats::RecordUseItem(const StatsUID& uid, not_null_ptr<ItemInstance> item)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL || item.get() == NULL)
        return;

    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    Item* itemType = item->getItem();
    if (itemType == NULL)
        return;

    round->mItemsUsed.push_back(itemType);
    if (itemType == Items::potion)
        ++round->mPotionsUsed;
    else if (itemType == Items::splashPotion)
        ++round->mSplashPotionsUsed;
}
void GameStats::RecordTimeOnLayer(const StatsUID& uid, int layer, bool start)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL || layer == -1)
        return;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    while (round->mLayerTime.size() <= static_cast<std::size_t>(layer))
        round->mLayerTime.push_back(0);
    ++round->mLayerTime.at(static_cast<std::size_t>(layer));
    if (start)
        ++round->mLayerTimeCounter;
}
void GameStats::RecordSnowballDirectHit(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        ++stats->mSnowballDirectHits;
}
void GameStats::RecordHitBySnowball(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        ++stats->mHitsFromProjectile;
}
void GameStats::RecordLanding(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL || mLandingRecorded)
        return;
    mLandingRecorded = true;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    round->mReachedEnd = true;
}
void GameStats::RecordBlockPlaced(const StatsUID& uid, const BlockPos&, int state) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) {
        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        ++round->mBlocksPlaced;
        ++round->mBlocksPlacedInfo.mCount;
        ++round->mBlocksPlacedInfo.mItems[state];
    }
}
void GameStats::RecordBlockDestroyed(const StatsUID& uid, const BlockPos&, int state) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) {
        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        ++round->mBlocksDestroyed.mCount;
        ++round->mBlocksDestroyed.mItems[state];
    }
}
unsigned int GameStats::RecordLastCheckpoint(const StatsUID&, int checkpoint, int, bool)
{
    if (static_cast<unsigned int>(checkpoint) > mFarthestCheckpointReached)
        mFarthestCheckpointReached = checkpoint;
    return static_cast<unsigned int>(checkpoint);
}
void GameStats::RecordLapCompleted(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
    {
        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        ++round->mLapsCompleted;
    }
}
unsigned int GameStats::GetLapsCompleted(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return 0;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    return static_cast<unsigned int>(round->mLapsCompleted);
}
unsigned int GameStats::GetLastCheckpointID(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0 : static_cast<unsigned int>(stats->getRoundStats(mCurrentRound)->mLastCheckpointId);
}
unsigned int GameStats::GetLastCheckpointSequenceID(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0 : static_cast<unsigned int>(stats->getRoundStats(mCurrentRound)->mLastCheckpointSequenceId);
}
void GameStats::RecordThermalEntered(const StatsUID& uid) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) ++stats->getRoundStats(mCurrentRound)->mThermalsEntered[0];
}
void GameStats::RecordProgress(const StatsUID& uid, double x, double y, double z, double progress) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) {
        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        round->mProgressStats[0] = static_cast<int>(x);
        round->mProgressStats[1] = static_cast<int>(y);
        round->mProgressStats[2] = static_cast<int>(z);
        round->mCurrentTime = progress;
    }
}
void GameStats::RecordSpawnedAtCheckpoint(const StatsUID& uid, int checkpoint) {
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL) stats->getRoundStats(mCurrentRound)->mSpawnedCheckpoints.insert(checkpoint);
}
void GameStats::RecordTimeSpentInPosition(bool) { }
unsigned int GameStats::RecordEndZoneEntered(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return 0;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    if (round->mReachedEnd)
        return 0;
    round->mReachedEnd = true;
    round->mLifeSpan = static_cast<int>(System::processTimeInMilliSecs() - mRoundStartTime);
    return 1;
}
unsigned char GameStats::GetEndZoneEntered(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0 : static_cast<unsigned char>(stats->getRoundStats(mCurrentRound)->mReachedEnd);
}
boost::shared_ptr<ServerPlayer> GameStats::GetLeadingPlayer() { return boost::shared_ptr<ServerPlayer>(); }
boost::shared_ptr<ServerPlayer> GameStats::GetTrailingPlayer(const boost::shared_ptr<ServerPlayer>& player) { return player; }
void GameStats::increasePlayerRoundStartTimeMs(long long milliseconds) { mRoundStartTime += static_cast<unsigned int>(milliseconds); }
long long GameStats::GetStartTime() { return mRoundStartTime; }
boost::shared_ptr<ServerPlayer> GameStats::GetRoundWinner() { return mRoundWinner; }
boost::shared_ptr<ServerPlayer> GameStats::GetSecondPlace() { return mSecondPlace; }
boost::shared_ptr<ServerPlayer> GameStats::GetThirdPlace() { return mThirdPlace; }
std::wstring GameStats::GetWinningThemeChoice() const { return std::wstring(); }
unsigned int GameStats::GetRoundsWon(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    return stats == NULL ? 0 : static_cast<unsigned int>(stats->getTotalRoundsWon());
}
unsigned int GameStats::GetMostBlocksDestroyed()
{
    unsigned int result = 0;
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        if (it->second != NULL)
            result = std::max(result, static_cast<unsigned int>(it->second->getTotalBlocksDestroyed()));
    }
    return result;
}
unsigned int GameStats::GetSecondMostBlocksDestroyed()
{
    unsigned int first = 0;
    unsigned int second = 0;
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        if (it->second == NULL)
            continue;
        const unsigned int value = static_cast<unsigned int>(it->second->getTotalBlocksDestroyed());
        if (value > first)
        {
            second = first;
            first = value;
        }
        else if (value > second)
            second = value;
    }
    return second;
}
unsigned int GameStats::GetThirdMostBlocksDestroyed()
{
    unsigned int first = 0;
    unsigned int second = 0;
    unsigned int third = 0;
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        if (it->second == NULL)
            continue;
        const unsigned int value = static_cast<unsigned int>(it->second->getTotalBlocksDestroyed());
        if (value > first)
        {
            third = second;
            second = first;
            first = value;
        }
        else if (value > second)
        {
            third = second;
            second = value;
        }
        else if (value > third)
            third = value;
    }
    return third;
}
unsigned int GameStats::GetFastestPlayerTime() { return GetShortestLifespan(mCurrentRound); }
unsigned int GameStats::GetHighestScoringPlayerScore()
{
    unsigned int result = 0;
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        if (it->second != NULL)
            result = std::max(result, static_cast<unsigned int>(it->second->getBestPoints(true, false)));
    }
    return result;
}
unsigned int GameStats::GetFarthestCheckpointReached() { return mFarthestCheckpointReached; }
